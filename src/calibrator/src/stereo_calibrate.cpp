#include "helpers/calibration_settings.h"
#include "helpers/calibrator.h"

int main(int argc, char* argv[]) {
    // The first of the two configs should always be for a camera that
    // already knows it's global position
    const string config1 = argc > 1 ? argv[1] : "/home/peter/Documents/Honours/ProjectEagle/src/client/config/devices/origin/extrinsic_conf.xml";
    const string config2 = argc > 2 ? argv[2] : "/home/peter/Documents/Honours/ProjectEagle/src/client/config/devices/next/extrinsic_conf.xml";

    cout << "Running stereo calibration test for configs: "
         << config1 << ", "
         << config2
         << "\n";

    // Calibrate the first camera
    Calibrator cal1(config1);
    cal1.execute();
    // cal1.saveCameraParams();

    // Calibrate the second camera
    Calibrator cal2(config2);
    cal2.execute();
    // cal2.saveCameraParams();

    // TODO we could get cameraMatrix, distCoeffs from cal file (from intrinsic calibration)

    // Stereo calibration
    // T2 = R12 * T1 + T12
    // R2 = R12 * R1
    Mat R12; Mat T12; Mat F; Mat E;
    stereoCalibrate(cal1.objectPoints,
                    cal1.imagePoints,
                    cal2.imagePoints,
                    cal1.cameraMatrix, cal1.distCoeffs, cal2.cameraMatrix, cal2.distCoeffs, cal1.imageSize,
                    R12, T12, F, E
    );

    // Get the settings ready
    CalSettings settings_integrated;
    settings_integrated.read(config1);

    CalSettings settings_new;
    settings_new.read(config2);

    // Load the extrinsic matrices of the already intrinsic device
    Mat R10; Mat T10;
    cv::FileStorage fs1(settings_integrated.outputFileName, cv::FileStorage::READ);
    fs1["rotation_matrix"] >> R10;
    fs1["translation_matrix"] >> T10;
    fs1.release();

    // We need to calculate R20 and T20
    // So that we can calculate T0 and R0 when we measure T2 and R2
    // T0 = R20 * T2 + T20
    // R0 = R20 * R2
    Mat R20; Mat T20;
    R20 = R10*R12.inv();
    T20 = T10 - R10*R12.inv()*T12;

    FileStorage fs( settings_new.outputFileName, FileStorage::WRITE );

    cout << R20 << "\n";
    cout << T20 << "\n";

    fs << "rotation_matrix" << R20;
    fs << "translation_matrix" << T20;

    fs.release();
}
