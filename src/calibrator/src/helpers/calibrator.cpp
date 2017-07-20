//
// Created by peter on 06/07/17.
//

#include "calibration_settings.h"
#include "calibrator.h"

Calibrator::Calibrator(string settings_path) : executed(false) {
    _settings.read(settings_path);
};

bool Calibrator::execute() {

    // Gather images_ceil1
    for (int i=0; i<_settings.imageCount; i++) {
        Mat view = getNextImage();
        // TODO what do we do for camera intake, when this takes a while? => does getNextImage freeze the execution?

        if (view.empty())
            // TODO do something
            continue;

        imageSize = view.size();

        // Process the image
        vector<Point2f> imageBuf;
        bool success = processImage(view, imageBuf);
        if (success)
            imagePoints.push_back(imageBuf);
        else
            continue;

        // Gather the object points
        // TODO this can be more efficient, but is the way it is now for flexibility
        vector<Point3f> objectBuf;
        success = processPattern(objectBuf);
        if (success)
            objectPoints.push_back(objectBuf);
        else
            // TODO we should ditch imagePoints, but this should never happen right now
            continue;
    }

    // TODO check for success

    // Get the intrinsic camera parameters
    getCalibration();

    // Calculate the reprojection errors
    computeReprojectionErrors();

    executed = true;
    return true;
}

Mat Calibrator::getNextImage() {
    Mat result;
    // Check the source of the images_ceil1
    switch(_settings.sourceType) {
        case CalSettings::SourceType::STORED:
            // We can get an image from the provided file
            if (_imageIndex < (int) _settings.imageList.size()) {
                string temp = _settings.imageList[_imageIndex++];
                result = imread(temp, CV_LOAD_IMAGE_COLOR);
            }
            break;
        default:
            break;
    }

    return result;
};

bool Calibrator::processImage(Mat view, vector<Point2f> &pointBuf) {
    // Flip the image if requested
    // if( _settings.flipVertical )
    //     flip( view, view, 0 );

    // Process image based on format
    bool found;
    switch( _settings.boardSettings.calibrationPattern ) {
        case BoardSettings::CHESSBOARD:
            found = findChessboardCorners( view, _settings.boardSettings.boardSize, pointBuf, 0);
                                            // CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);
            break;
        case BoardSettings::CIRCLES_GRID:
            found = findCirclesGrid( view, _settings.boardSettings.boardSize, pointBuf );
            break;
        case BoardSettings::ASYMMETRIC_CIRCLES_GRID:
            found = findCirclesGrid( view, _settings.boardSettings.boardSize, pointBuf, CALIB_CB_ASYMMETRIC_GRID );
            break;
        default:
            found = false;
            break;
    }

    if (found) {
        // improve the found corners' coordinate accuracy for chessboard
        if( _settings.boardSettings.calibrationPattern == BoardSettings::CHESSBOARD) {
            Mat viewGray;
            cvtColor(view, viewGray, COLOR_BGR2GRAY);
            cornerSubPix( viewGray, pointBuf, _settings.boardSettings.boardSize,
                          Size(-1,-1), TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));
        }

        // Draw the corners.
        drawChessboardCorners( view, _settings.boardSettings.boardSize, Mat(pointBuf), found );
    }

    imshow("Image View", view);
    (char)waitKey(500);

    return found;
};

bool Calibrator::processPattern(vector<Point3f> &pointBuf) {
    // Reset the corners
    pointBuf.clear();

    // Calculate grid points based on pattern
    switch(_settings.boardSettings.calibrationPattern) {
        case BoardSettings::CHESSBOARD:
        case BoardSettings::CIRCLES_GRID:
            for( int i = 0; i < _settings.boardSettings.boardSize.height; ++i )
                for( int j = 0; j < _settings.boardSettings.boardSize.width; ++j )
                    pointBuf.push_back(Point3f(float( j*_settings.boardSettings.squareSize ), float( i*_settings.boardSettings.squareSize ), 0));
            break;

        case BoardSettings::ASYMMETRIC_CIRCLES_GRID:
            for( int i = 0; i < _settings.boardSettings.boardSize.height; i++ )
                for( int j = 0; j < _settings.boardSettings.boardSize.width; j++ )
                    pointBuf.push_back(Point3f(float((2*j + i % 2)*_settings.boardSettings.squareSize), float(i*_settings.boardSettings.squareSize), 0));
            break;
        default:
            break;
    }

    return true;
};

bool Calibrator::getCalibration() {

    // Initialise CameraMatrix
    cameraMatrix = Mat::eye(3, 3, CV_64F);
    if( _settings.flag & CV_CALIB_FIX_ASPECT_RATIO )
        cameraMatrix.at<double>(0,0) = 1.0;

    // Initialise Distortion Coefficients
    distCoeffs = Mat::zeros(8, 1, CV_64F);

    //Find intrinsic and extrinsic camera parameters
    calibrateCamera(objectPoints, imagePoints, imageSize, cameraMatrix,
                                 distCoeffs, rvecs, tvecs, _settings.flag|CV_CALIB_FIX_K4|CV_CALIB_FIX_K5);

    // Check the result
    bool success = checkRange(cameraMatrix) && checkRange(distCoeffs);
    return success;
};

double Calibrator::computeReprojectionErrors() {

    vector<Point2f> imagePoints2;
    int i, totalPoints = 0;
    double totalErr = 0, err;
    reprojErrs.resize(objectPoints.size());

    for( i = 0; i < (int)objectPoints.size(); ++i ) {
        projectPoints( Mat(objectPoints[i]), rvecs[i], tvecs[i], cameraMatrix,
                       distCoeffs, imagePoints2);
        err = norm(Mat(imagePoints[i]), Mat(imagePoints2), CV_L2);

        int n = (int)objectPoints[i].size();
        reprojErrs[i] = (float) std::sqrt(err*err/n);
        totalErr        += err*err;
        totalPoints     += n;
    }

    return std::sqrt(totalErr/totalPoints);
};

void Calibrator::saveCameraParams() {

    if (!executed) {
        cerr << "can\'t store calibration that has not been executed yet";
        return;
    }

    CalSettings s = _settings;

    FileStorage fs( s.outputFileName, FileStorage::WRITE );

    time_t tm;
    time( &tm );
    struct tm *t2 = localtime( &tm );
    char buf[1024];
    strftime( buf, sizeof(buf)-1, "%c", t2 );

    fs << "calibration_Time" << buf;

    if( !rvecs.empty() || !reprojErrs.empty() )
        fs << "nrOfFrames" << (int)std::max(rvecs.size(), reprojErrs.size());
    fs << "image_Width" << imageSize.width;
    fs << "image_Height" << imageSize.height;
    fs << "board_Width" << s.boardSettings.boardSize.width;
    fs << "board_Height" << s.boardSettings.boardSize.height;
    fs << "square_Size" << s.boardSettings.squareSize;

    if( s.flag & CV_CALIB_FIX_ASPECT_RATIO )
        fs << "FixAspectRatio" << s.aspectRatio;

    if( s.flag ) {
        sprintf( buf, "flags: %s%s%s%s",
                 s.flag & CV_CALIB_USE_INTRINSIC_GUESS ? " +use_intrinsic_guess" : "",
                 s.flag & CV_CALIB_FIX_ASPECT_RATIO ? " +fix_aspectRatio" : "",
                 s.flag & CV_CALIB_FIX_PRINCIPAL_POINT ? " +fix_principal_point" : "",
                 s.flag & CV_CALIB_ZERO_TANGENT_DIST ? " +zero_tangent_dist" : "" );
        cvWriteComment( *fs, buf, 0 );

    }

    fs << "flagValue" << s.flag;

    fs << "camera_matrix" << cameraMatrix;
    fs << "distortion_vector" << distCoeffs;

    fs << "Avg_Reprojection_Error" << totalAvgErr;
    if( !reprojErrs.empty() )
        fs << "Per_View_Reprojection_Errors" << Mat(reprojErrs);

    if( !rvecs.empty() && !tvecs.empty() ) {
        CV_Assert(rvecs[0].type() == tvecs[0].type());
        Mat bigmat((int)rvecs.size(), 6, rvecs[0].type());
        for( int i = 0; i < (int)rvecs.size(); i++ ) {
            Mat r = bigmat(Range(i, i+1), Range(0,3));
            Mat t = bigmat(Range(i, i+1), Range(3,6));

            CV_Assert(rvecs[i].rows == 3 && rvecs[i].cols == 1);
            CV_Assert(tvecs[i].rows == 3 && tvecs[i].cols == 1);
            //*.t() is MatExpr (not Mat) so we can use assignment operator
            r = rvecs[i].t();
            t = tvecs[i].t();
        }
        cvWriteComment( *fs, "a set of 6-tuples (rotation vector + translation vector) for each view", 0 );
        fs << "Extrinsic_Parameters" << bigmat;
    }

    if( !imagePoints.empty() ) {
        Mat imagePtMat((int)imagePoints.size(), (int)imagePoints[0].size(), CV_32FC2);
        for( int i = 0; i < (int)imagePoints.size(); i++ ) {
            Mat r = imagePtMat.row(i).reshape(2, imagePtMat.cols);
            Mat imgpti(imagePoints[i]);
            imgpti.copyTo(r);
        }
        fs << "Image_points" << imagePtMat;
    }

    fs.release();
};