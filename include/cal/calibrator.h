#ifndef PROJECTEAGLE_CALIBRATOR_H
#define PROJECTEAGLE_CALIBRATOR_H

#include "calibration_settings.h"
#include "board_settings.h"

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.hpp>
#include <iostream>

using namespace cv;
using namespace std;


class Calibrator {

public:
    CalSettings _settings;
    int _imageIndex = 0;
    Size imageSize;

    vector<vector<Point2f>> imagePoints;
    vector<vector<Point3f>> objectPoints;

    vector<Mat> rvecs;
    vector<Mat> tvecs;
    Mat cameraMatrix;
    Mat distCoeffs;

    double totalAvgErr;
    vector<float> reprojErrs;

public:
    Calibrator(CalSettings s);

    bool execute();
    void saveCameraParams();

private:
    bool executed;

    Mat getNextImage();

    bool processImage(Mat view, vector<Point2f> &pointBuf);
    bool processPattern(vector<Point3f> &pointBuf);

    bool getCalibration();

    double computeReprojectionErrors();
};


#endif //PROJECTEAGLE_CALIBRATOR_H
