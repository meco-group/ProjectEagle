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

private:
    CalSettings _settings;
    int _imageIndex = 0;

public:
    Calibrator(CalSettings s);

private:
    Mat getNextImage();
    bool processImage(Mat view, vector<Point2f> &pointBuf);
    bool processPattern(vector<Point3f> &pointBuf);
};


#endif //PROJECTEAGLE_CALIBRATOR_H
