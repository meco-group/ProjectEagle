#include "cal/board_settings.h"
#include "cal/calibration_settings.h"
#include "cal/calibrator.h"
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

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
    FileStorage fs("board_settings.xml", FileStorage::READ);
    BoardSettings boardSettings;
    fs["BoardSettings"] >> boardSettings;
    fs.release();

    CalSettings calSettings;
    calSettings.sourceType = CalSettings::SourceType::STORED;

    calSettings.imageCount = 10;
    calSettings.imageList = {
            "images/0.jpg",
            "images/1.jpg",
            "images/2.jpg",
            "images/3.jpg",
            "images/4.jpg",
            "images/5.jpg",
            "images/6.jpg",
            "images/7.jpg",
            "images/8.jpg",
            "images/9.jpg"
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

    calSettings.outputFileName = "test_cal.xml";

    calSettings.aspectRatio = 1.0;

    Calibrator cal(calSettings);
    cal.execute();
    cal.saveCameraParams();
}