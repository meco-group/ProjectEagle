//
// Created by peter on 04/07/17.
//

#ifndef PROJECTEAGLE_CALIBRATOR_H
#define PROJECTEAGLE_CALIBRATOR_H


#include "cal_settings.h"

class Calibrator {
    public:
        /**
         * Calibrate a camera
         * @param settingsFile  The settings used for calibration
         * @param cameraMatrix  The output camera matrix
         * @param distCoeffs    The output distortion coefficients
         * @throws invalid_argument invalid settingsFile path
         */
        static void calibrate(string settingsFile, Mat&  cameraMatrix, Mat& distCoeffs);

    private:
        /**
         * Load the Calibration Settings from provided file
         * @param settingsFile settings file path
         * @return CalSettings parsed from settingsFile
         * @throws invalid_argument invalid settingsFile path
         */
        static CalSettings loadSettings(string settingsFile);
};


#endif //PROJECTEAGLE_CALIBRATOR_H
