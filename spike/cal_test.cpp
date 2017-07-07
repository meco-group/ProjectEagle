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