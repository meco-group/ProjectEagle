//
// Created by peter on 04/07/17.
//

#include <opencv/cv.hpp>
#include "cal/calibrator.h"

void Calibrator::calibrate(string settingsFile, Mat &cameraMatrix, Mat &distCoeffs) {
    // Load the settings file
    CalSettings s = loadSettings(settingsFile);

    // Get the mode that the calibration is currently operating in
    //  false:   We load frames until empty
    //  true:    We load frames until we have as many frames as the settings specify are required
    int count_frames = s.inputType == CalSettings::IMAGE_LIST;

    vector<vector<Point2f> > imagePoints;   // The image points we are gathering
    Size imageSize;                         // The size of the image
    clock_t prevTimestamp = 0;              // Timing information

    // Loop through all images and gather imagePoints
    for (int i = 0;;++i) {
        // Load the next image from the CalSettings
        Mat view = s.nextImage();

        // If we are using a video we only take one frame per s.delay
        if (!s.inputCapture.isOpened() || clock() - prevTimestamp > s.delay*1e-3*CLOCKS_PER_SEC)
            prevTimestamp = clock();    // Reset the timer
        else
            continue;                   // Skip this loop iteration

        // Check if we finished calibrating based on the number of frames we have already
        if (count_frames && imagePoints.size() >= (unsigned)s.nrFrames) {
            count_frames = runCalibrationAndSave(s, imageSize,  cameraMatrix, distCoeffs, imagePoints);
            if (count_frames)
                break;
        }

        // There are no more frames coming in, so we have to finish
        else if (view.empty()) {
            if (imagePoints.size() > 0)
                count_frames = runCalibrationAndSave(s, imageSize, cameraMatrix, distCoeffs, imagePoints);
            break;
        }

        // Process the image
        imageSize = view.size();
        processImage(s, view, imagePoints);
    }

    if (!count_frames)
        throw invalid_argument("the provided images were not sufficient to calibrate the camera");
}

CalSettings Calibrator::loadSettings(string settingsFile) {
    CalSettings s;
    FileStorage fs(settingsFile, FileStorage::READ);
    if (!fs.isOpened()) {
        throw invalid_argument("settingsFile at "+settingsFile+" could not be opened");
    }

    fs["Settings"] >> s;
    fs.release();

    if (!s.goodInput) {
        throw invalid_argument("invalid settingsfile "+settingsFile);
    }

    return s;
}

void Calibrator::processImage(CalSettings s, Mat view, vector<vector<Point2f>> &imagePoints) {
    // Flip the image if requested
    if( s.flipVertical )
        flip( view, view, 0 );

    vector<Point2f> pointBuf;

    // Process image based on format
    bool found;
    switch( s.calibrationPattern ) {
        case CalSettings::CHESSBOARD:
            found = findChessboardCorners( view, s.boardSize, pointBuf,
                                           CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);
            break;
        case CalSettings::CIRCLES_GRID:
            found = findCirclesGrid( view, s.boardSize, pointBuf );
            break;
        case CalSettings::ASYMMETRIC_CIRCLES_GRID:
            found = findCirclesGrid( view, s.boardSize, pointBuf, CALIB_CB_ASYMMETRIC_GRID );
            break;
        default:
            found = false;
            break;
    }

    if (found) {
        // improve the found corners' coordinate accuracy for chessboard
        if( s.calibrationPattern == CalSettings::CHESSBOARD) {
            Mat viewGray;
            cvtColor(view, viewGray, COLOR_BGR2GRAY);
            cornerSubPix( viewGray, pointBuf, Size(11,11),
                          Size(-1,-1), TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));
        }

        // add the calculated points
        imagePoints.push_back(pointBuf);
    }
}

int Calibrator::runCalibrationAndSave(CalSettings s, Size imageSize, Mat &cameraMatrix, Mat &distCoeffs,
                                      vector<vector<Point2f>> imagePoints) {

    // Initialise CameraMatrix
    cameraMatrix = Mat::eye(3, 3, CV_64F);
    if( s.flag & CV_CALIB_FIX_ASPECT_RATIO )
        cameraMatrix.at<double>(0,0) = 1.0;

    // Initialise Distortion Coefficients
    distCoeffs = Mat::zeros(8, 1, CV_64F);

    // Calculate the object points
    vector<vector<Point3f>> objectPoints(1);
    calcBoardCornerPositions(s.boardSize, s.squareSize, objectPoints[0], s.calibrationPattern);
    objectPoints.resize(imagePoints.size(),objectPoints[0]);

    //Find intrinsic and extrinsic camera parameters
    vector<Mat> rvecs, tvecs;
    vector<float> reprojErrs;
    double totalAvgErr = 0;
    double rms = calibrateCamera(objectPoints, imagePoints, imageSize, cameraMatrix,
                                 distCoeffs, rvecs, tvecs, s.flag|CV_CALIB_FIX_K4|CV_CALIB_FIX_K5);

    // Check the result
    bool success = checkRange(cameraMatrix) && checkRange(distCoeffs);

    // Calculate the error
    totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints,
                                            rvecs, tvecs, cameraMatrix, distCoeffs, reprojErrs);

    // Store the result
    if (success) {
        saveCameraParams( s, imageSize, cameraMatrix, distCoeffs, rvecs ,tvecs, reprojErrs,
                          imagePoints, totalAvgErr);
    }

    return success;
}

void Calibrator::calcBoardCornerPositions(Size boardSize, float squareSize, vector<Point3f> &corners,
                                          CalSettings::Pattern patternType) {
    // Reset the corners
    corners.clear();

    // Calculate grid points based on pattern
    switch(patternType)
    {
        case CalSettings::CHESSBOARD:
        case CalSettings::CIRCLES_GRID:
            for( int i = 0; i < boardSize.height; ++i )
                for( int j = 0; j < boardSize.width; ++j )
                    corners.push_back(Point3f(float( j*squareSize ), float( i*squareSize ), 0));
            break;

        case CalSettings::ASYMMETRIC_CIRCLES_GRID:
            for( int i = 0; i < boardSize.height; i++ )
                for( int j = 0; j < boardSize.width; j++ )
                    corners.push_back(Point3f(float((2*j + i % 2)*squareSize), float(i*squareSize), 0));
            break;
        default:
            break;
    }
}

double Calibrator::computeReprojectionErrors(vector<vector<Point3f>> objectPoints, vector<vector<Point2f>> imagePoints,
                                             vector<Mat> rvecs, vector<Mat> tvecs, Mat &cameraMatrix, Mat &distCoeffs,
                                             vector<float> perViewErrors) {
    vector<Point2f> imagePoints2;
    int i, totalPoints = 0;
    double totalErr = 0, err;
    perViewErrors.resize(objectPoints.size());

    for( i = 0; i < (int)objectPoints.size(); ++i ) {
        projectPoints( Mat(objectPoints[i]), rvecs[i], tvecs[i], cameraMatrix,
                       distCoeffs, imagePoints2);
        err = norm(Mat(imagePoints[i]), Mat(imagePoints2), CV_L2);

        int n = (int)objectPoints[i].size();
        perViewErrors[i] = (float) std::sqrt(err*err/n);
        totalErr        += err*err;
        totalPoints     += n;
    }

    return std::sqrt(totalErr/totalPoints);
}

void Calibrator::saveCameraParams( CalSettings& s, Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs,
                                   const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                                   const vector<float>& reprojErrs, const vector<vector<Point2f> >& imagePoints,
                                   double totalAvgErr ) {

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
    fs << "board_Width" << s.boardSize.width;
    fs << "board_Height" << s.boardSize.height;
    fs << "square_Size" << s.squareSize;

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

    fs << "Camera_Matrix" << cameraMatrix;
    fs << "Distortion_Coefficients" << distCoeffs;

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
}
