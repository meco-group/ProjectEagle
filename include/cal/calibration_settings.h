//
// Created by peter on 06/07/17.
//

#ifndef PROJECTEAGLE_CALIBRATION_SETTINGS_H
#define PROJECTEAGLE_CALIBRATION_SETTINGS_H

#include "board_settings.h"

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

class CalSettings {
public:
    // The source of the images
    enum SourceType {NONE, STORED, VIDEO, CAMERA};
    SourceType sourceType;

    // Amount of frames to take
    int imageCount;

    // Data for case of STORED
    vector<string> imageList;

    // The board settings
    BoardSettings boardSettings;

    // Input settings
    bool flipVertical;

    // The Calibration Flag
    int flag;

    // The output file name
    string outputFileName;

    // The aspect ratio
    float aspectRatio;
};


#endif //PROJECTEAGLE_CALIBRATION_SETTINGS_H
