#include "experiments.h"

using namespace cv;
using namespace std;

void gen_settings() {
    // Generate the settings file
    BoardSettings s;
    s.boardSize = Size(9, 6);
    s.squareSize = 25;
    s.calibrationPattern = s.CHESSBOARD;

    // Open and write file
    FileStorage fs("test.xml", FileStorage::WRITE);
    s.write(fs);
    fs.release();
}

void cal_test() {
    // Open settings file
    FileStorage fs("../config/ceil1_cam.xml", FileStorage::READ);
    BoardSettings boardSettings;
    fs["BoardSettings"] >> boardSettings;
    fs.release();

    CalSettings calSettings;
    calSettings.sourceType = CalSettings::SourceType::STORED;

    calSettings.imageCount = 10;
    calSettings.imageList = {
            "../config/images_ceil1/0.jpg",
            "../config/images_ceil1/1.jpg",
            "../config/images_ceil1/2.jpg",
            "../config/images_ceil1/3.jpg",
            "../config/images_ceil1/4.jpg",
            "../config/images_ceil1/5.jpg",
            "../config/images_ceil1/6.jpg",
            "../config/images_ceil1/7.jpg",
            "../config/images_ceil1/8.jpg",
            "../config/images_ceil1/9.jpg"
    };
    calSettings.boardSettings = boardSettings;
    calSettings.flipVertical = false;

    bool calibFixPrincipalPoint = true;
    bool calibZeroTangentDist = true;
    bool aspectRatio = true;

    int flag = 0;
    if(calibFixPrincipalPoint) flag |= CV_CALIB_FIX_PRINCIPAL_POINT;
    if(calibZeroTangentDist)   flag |= CV_CALIB_ZERO_TANGENT_DIST;
    if(aspectRatio)            flag |= CV_CALIB_FIX_ASPECT_RATIO;
    calSettings.flag = flag;

    calSettings.outputFileName = "../config/ceil1_cam_cal.xml";

    calSettings.aspectRatio = 1.0;

    Calibrator cal(calSettings);
    cal.execute();
    cal.saveCameraParams();
}

void stereo_cal() {

    // Calibrate the first camera
    // Open settings file
    FileStorage fs("../config/ceil1_cam.xml", FileStorage::READ);
    BoardSettings boardSettings;
    fs["BoardSettings"] >> boardSettings;
    fs.release();

    CalSettings cal1Settings;
    cal1Settings.sourceType = CalSettings::SourceType::STORED;

    cal1Settings.imageCount = 5;
    cal1Settings.imageList = {
            "../config/images_ceil1/0.jpg",
            "../config/images_ceil1/1.jpg",
            "../config/images_ceil1/2.jpg",
            "../config/images_ceil1/3.jpg",
            "../config/images_ceil1/4.jpg"
    };
    cal1Settings.boardSettings = boardSettings;
    cal1Settings.flipVertical = false;

    bool calibFixPrincipalPoint = true;
    bool calibZeroTangentDist = true;
    bool aspectRatio = true;

    int flag = 0;
    if(calibFixPrincipalPoint) flag |= CV_CALIB_FIX_PRINCIPAL_POINT;
    if(calibZeroTangentDist)   flag |= CV_CALIB_ZERO_TANGENT_DIST;
    if(aspectRatio)            flag |= CV_CALIB_FIX_ASPECT_RATIO;
    cal1Settings.flag = flag;

    cal1Settings.outputFileName = "../config/ceil1_cam_cal.xml";

    cal1Settings.aspectRatio = 1.0;

    Calibrator cal1(cal1Settings);
    cal1.execute();
    cal1.saveCameraParams();

    // Calibrate the second camera

    CalSettings cal2Settings;
    cal2Settings.sourceType = CalSettings::SourceType::STORED;

    cal2Settings.imageCount = 10;
    cal2Settings.imageList = {
            "../config/images_ceil2/0.jpg",
            "../config/images_ceil2/1.jpg",
            "../config/images_ceil2/2.jpg",
            "../config/images_ceil2/3.jpg",
            "../config/images_ceil2/4.jpg",
            "../config/images_ceil2/5.jpg",
            "../config/images_ceil2/6.jpg",
            "../config/images_ceil2/7.jpg",
            "../config/images_ceil2/8.jpg",
            "../config/images_ceil2/9.jpg"
    };
    cal2Settings.boardSettings = boardSettings;
    cal2Settings.flipVertical = false;

    cal2Settings.outputFileName = "../config/ceil2_cam_cal.xml";

    cal2Settings.aspectRatio = 1.0;

    Calibrator cal2(cal2Settings);
    cal2.execute();
    cal2.saveCameraParams();

    Mat R; Mat T; Mat F; Mat E;
    stereoCalibrate(vector<vector<Point3f>>(&cal1.objectPoints[0],&cal1.objectPoints[4]),
                    vector<vector<Point2f>>(&cal1.imagePoints[0],&cal1.imagePoints[4]),
                    vector<vector<Point2f>>(&cal2.imagePoints[0],&cal2.imagePoints[4]),
                    cal1.cameraMatrix, cal1.distCoeffs, cal2.cameraMatrix, cal2.distCoeffs, cal1.imageSize,
                    R, T, F, E
    );

    cout << R << "\n";
    cout << T << "\n";
}