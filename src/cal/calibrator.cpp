//
// Created by peter on 04/07/17.
//

#include "cal/calibrator.h"

void Calibrator::calibrate(string settingsFile, Mat &cameraMatrix, Mat &distCoeffs) {
    // Load the settings file
    CalSettings s = loadSettings(settingsFile);

    // Loop through all images
    for (int i = 0;;++i) {
        // Load the next image from the CalSettings
        Mat view = s.nextImage();

        // TODO continue here

        break;
    }
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
