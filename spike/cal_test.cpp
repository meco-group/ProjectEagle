#include "experiments.h"

using namespace cv;
using namespace std;

void gen_settings() {
    // Generate the board settings
    BoardSettings sboard;
    sboard.boardSize = Size(7, 6);
    sboard.squareSize = 108;
    sboard.calibrationPattern = sboard.CHESSBOARD;

    // Generate the camera settings
    CameraSettings scam;
    scam.calPath = "../config/ceil1_cam_cal.xml";
    scam.camIndex = 1;
    scam.camType = SEE3CAM;

    // Generate the calibration settings
    CalSettings scal;
    scal.sourceType = scal.STORED;
    scal.imageCount = 10;
    scal.sourcePath = "../config/ceil1_images.xml";
    scal.boardSettingsPath = "../config/ceil1_cam.xml";
    scal.calibFixPrincipalPoint = false;
    scal.calibZeroTangentDist = false;
    scal.aspectRatio = 1;
    scal.outputFileName = "../config/ceil1_cam_cal.xml";

    // Generate the comm settings
    CommSettings scom;
    scom.interface = "wlp58s0";
    scom.init_wait_time = 100;

    // Open and write file
    FileStorage fs("../config/ceil1_cam.xml", FileStorage::WRITE);
    scam.write(fs);
    sboard.write(fs);
    scal.write(fs);
    scom.write(fs);
    fs.release();
}

void cal_test(string config) {
    cout << "Running calibration test for config: " << config << "\n";
    // Open settings file
    FileStorage fs(config, FileStorage::READ);
    CalSettings calSettings;
    fs["CalibrationSettings"] >> calSettings;
    fs.release();

    // Calibrate
    Calibrator cal(calSettings);
    cal.execute();
    cal.saveCameraParams();
}

void stereo_cal(string config1, string config2) {
    cout << "Running stereo calibration test for configs: "
         << config1 << ", "
         << config2
         << "\n";

    // Calibrate the first camera
    // Open settings file
    FileStorage fs1(config1, FileStorage::READ);
    CalSettings cal1Settings;
    fs1["CalibrationSettings"] >> cal1Settings;
    fs1.release();

    Calibrator cal1(cal1Settings);
    cal1.execute();
    // cal1.saveCameraParams();

    // Calibrate the second camera
    // Open settings file
    FileStorage fs2(config2, FileStorage::READ);
    CalSettings cal2Settings;
    fs2["CalibrationSettings"] >> cal2Settings;
    fs2.release();

    Calibrator cal2(cal2Settings);
    cal2.execute();
    // cal2.saveCameraParams();

    // Stereo calibration
    Mat R; Mat T; Mat F; Mat E;
    stereoCalibrate(cal1.objectPoints,
                    cal1.imagePoints,
                    cal2.imagePoints,
                    cal1.cameraMatrix, cal1.distCoeffs, cal2.cameraMatrix, cal2.distCoeffs, cal1.imageSize,
                    R, T, F, E
    );

    cout << R << "\n";
    cout << T << "\n";
}