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

        /**
         * Run Calibration on the gathered imagePoints
         * @param settings      the calibration settings
         * @param imageSize     size of the images
         * @param cameraMatrix  output camera matrix
         * @param distCoeffs    output distortion coefficients
         * @param imagePoints   the image points
         * @return              calibration error
         */
        static int runCalibrationAndSave(CalSettings settings, Size imageSize, Mat &cameraMatrix, Mat &distCoeffs, vector<vector<Point2f>> imagePoints);

        /**
         * Process the image coming in from the calibration settings
         * @param settings      the calibration settings
         * @param view          the image to process
         * @param imagePoints   output image points for calibration
         */
        static void processImage(CalSettings settings, Mat view, vector<vector<Point2f>> &imagePoints);

        /**
         * Save the result of the camera calibration in a file
         * @param s             the calibration settings
         * @param imageSize     size of the images used for calibration
         * @param cameraMatrix  the camera matrix
         * @param distCoeffs    the distortion coefficients
         * @param rvecs         rotation vectors of the board
         * @param tvecs         translation vectors of the board
         * @param reprojErrs    per view reprojection errors
         * @param imagePoints   the image points of the board
         * @param totalAvgErr   the total average reprojection errors
         */
        static void saveCameraParams(CalSettings& s, Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs,
                                      const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                                      const vector<float>& reprojErrs, const vector<vector<Point2f> >& imagePoints,
                                      double totalAvgErr );

        /**
         * Calculate the board corner positions
         * @param boardSize     size of the board
         * @param squareSize    size of the squares on the board
         * @param corners       output corners
         * @param patternType   output pattern type
         */
        static void calcBoardCornerPositions(Size boardSize, float squareSize, vector<Point3f> &corners, CalSettings::Pattern patternType);

        /**
         * Evaluate the calibration
         * @param objectPoints  the object points of the board
         * @param imagePoints   the image points of the board
         * @param rvecs         rotation vectors of the board
         * @param tvecs         translation vectors of the board
         * @param cameraMatrix  the camera matrix
         * @param distCoeffs    the distortion coefficients
         * @param perViewErrors per view reprojection errors
         * @return
         */
        static double computeReprojectionErrors(vector<vector<Point3f>> objectPoints, vector<vector<Point2f>> imagePoints,
                                                vector<Mat> rvecs, vector<Mat> tvecs, Mat &cameraMatrix, Mat &distCoeffs,
                                                vector<float> perViewErrors);
};


#endif //PROJECTEAGLE_CALIBRATOR_H
