#include "calibration_settings.h"
#include "calibrator.h"

using namespace eagle;

Calibrator::Calibrator(string config_path) : executed(false) {
    _settings.read(config_path);
};

bool Calibrator::execute() {
    // Gather images_ceil1
    string filename;
    for (int i=0; i<_settings.imageCount; i++) {
        Mat view = getNextImage(filename);
        // TODO what do we do for camera intake, when this takes a while? => does getNextImage freeze the execution?

        if (view.empty())
            // TODO do something
            continue;

        imageSize = view.size();

        // Process the image
        vector<Point2f> imageBuf;
        bool success = processImage(view, imageBuf);
        if (success){
            imagePoints.push_back(imageBuf);
        } else {
            std::cout << "Ditch image: " << filename << std::endl;
        }
            //continue;

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
    rescaleTransformation(false);

    executed = true;
    return true;
}

Mat Calibrator::getNextImage(string &name) {
    Mat result;
    // Check the source of the images_ceil1
    switch(_settings.sourceType) {
        case CalSettings::SourceType::STORED:
            // We can get an image from the provided file
            if (_imageIndex < (int) _settings.imageList.size()) {
                name = _settings.imageList[_imageIndex++];
                result = imread(name, CV_LOAD_IMAGE_COLOR);
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

    calibrateCamera(objectPoints, imagePoints, imageSize, cameraMatrix,distCoeffs, rvecs, tvecs, _settings.flag|CV_CALIB_FIX_K4|CV_CALIB_FIX_K5);

    //Process the extrinsic camera parameters:
    //For now only translation is taken into account. Rotated images are not yet.
    //We still have to write a routine to compute the rotation between the image plane and the world plane.
    double Z = 0.0;
    for(int k=0; k<tvecs.size();k++){
        Z += tvecs[k].at<double>(2,0);
    }
    Z = Z/tvecs.size();


    groundPlane = Mat::zeros(1,4,CV_64F);
    groundPlane.at<double>(0,2) = 1.0;
    groundPlane.at<double>(0,3) = -Z;

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

double Calibrator::rescaleTransformation(bool apply = false) {

    double tsum = 0.0;
    int tcount = 0;

    for(int k = 0;k<imagePoints.size();k++){
    	Mat undistorted;
    	Mat world;
        undistortPoints(Mat(imagePoints[k]),undistorted,Mat::eye(3,3,CV_64F),distCoeffs);

    	std::vector<double> norms;

    	double sum = 0.0;
    	int count = 0;
    	for(int j = 1;j<imagePoints[k].size();j++){
            Point3d i1(imagePoints[k][j].x,imagePoints[k][j].y,1);
            Point3d P1;
            Point3d i0(imagePoints[k][j-1].x,imagePoints[k][j-1].y,1);
            Point3d P0;
            Calibrator::projectToGround(i1,P1,cameraMatrix,groundPlane);
            Calibrator::projectToGround(i0,P0,cameraMatrix,groundPlane);

//            std::cout << P0 << "," << P1 << std::endl;

    		double temp = cv::norm(Mat(P1),Mat(P0));
    		if(count==0 || (temp <= 1.2*(sum/count))){
    			sum += temp;
    			count++;
    		}
    	}

	    double average = sum/count;
	    std::cout << "Average square distance " << k << ": " << average << std::endl;

        tsum += sum;
        tcount += count;
    }
    double scale = tsum/tcount;
    std::cout << "Global average square distance: " << scale << std::endl;
    std::cout << "Relative error: " << (_settings.boardSettings.squareSize-scale)/_settings.boardSettings.squareSize << std::endl;

	return 0;
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
    fs << "ground_plane" << groundPlane;

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

void Calibrator::projectToGround(const Point3d &i, Point3d &w, Mat K, Mat ground)
{
    // Project image coordinates to a plane ground
    // i is of the form [x;y;1], w is of the form [X,Y,Z], K is the camera matrix
    // ground holds the coefficients of the ground plane [a,b,c,d] where the
    // ground plane is represented by ax+by+cz+d=0

    Mat M1;
    hconcat(-Mat::eye(3,3,CV_64F),K.inv()*Mat(i),M1);
    Mat M2;
    hconcat(ground(Rect(0,0,3,1)),Mat::zeros(1,1,CV_64F),M2);
    Mat A;
    vconcat(M1,M2,A);

    Mat b = Mat::zeros(4,1,CV_64F);
    b.at<double>(3,0) = -ground.at<double>(0,3);

    Mat W = A.inv()*b;
    w = Point3d(W(Rect(0,0,1,3)));
}

static void projectToImage(Point3d &i, const Point3d &w, Mat K)
{
    Mat p = K*Mat(w);
    p = p*(1.0/p.at<double>(2,0));
    i = Point3d(p.at<double>(0,0),p.at<double>(1,0),p.at<double>(2,0));
}
