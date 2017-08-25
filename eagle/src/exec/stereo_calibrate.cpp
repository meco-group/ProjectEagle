#include "calibrator.h"
#include "camera.h"
#include "utils.h"

using namespace eagle;

int main(int argc, char* argv[]) {
    std::string config_path1 = (argc > 1) ? argv[1] : CONFIG_PATH;
    std::string config_path2 = (argc > 2) ? argv[2] : CONFIG_PATH;
    std::string images_path1 = (argc > 3) ? argv[3] : CAL_IMAGES_PATH;
    std::string images_path2 = (argc > 4) ? argv[4] : CAL_IMAGES_PATH;

    // calibrate the first camera
    Calibrator cal1(config_path1);
    cal1.execute(images_path1);
    // cal1.save(config_path1);

    // calibrate the second camera
    Calibrator cal2(config_path1);
    cal2.execute(images_path2);
    // cal2.save(config_path2);

    // TODO we could get cameraMatrix, distCoeffs from cal file (from intrinsic calibration)
    cv::FileStorage fs1(config_path1, cv::FileStorage::READ);
    cv::FileStorage fs2(config_path2, cv::FileStorage::READ);
    cv::Mat K1, K2, D1, D2;
    fs1["camera"]["camera_matrix"] >> K1;
    fs1["camera"]["distortion_vector"] >> D1;
    fs2["camera"]["camera_matrix"] >> K2;
    fs2["camera"]["distortion_vector"] >> D2;
    fs2.release();
    fs1.release();

    // Stereo calibration
    // t2 = R12 * t1 + t12
    // R2 = R12 * R1
    cv::Mat R12, t12, T12, F, E, t2;
    cv::stereoCalibrate(cal1.object_points(), cal1.image_points(), cal2.image_points(), K1, D1, K2, D2, cal1.image_size(), R12, t12, F, E);
    std::cout << R12 << std::endl << t12 << std::endl;
    cv::hconcat(R12, t12, T12);
    cv::hconcat(cv::Mat::zeros(1, 3, CV_64F), cv::Mat::ones(1, 1, CV_64F), t2);
    cv::vconcat(T12, t2, T12);

    // Load the extrinsic matrices of the already intrinsic device
    cv::Mat T10;
    fs1 = cv::FileStorage(config_path1, cv::FileStorage::READ);
    fs1["external_transformation"] >> T10;
    fs1.release();

    // We need to calculate R20 and T20
    // So that we can calculate T0 and R0 when we measure T2 and R2
    // t0 = R20 * t2 + t20
    // R0 = R20 * R2
    // i.e. T0 = T20*T2
    // ruben doesn't get this??
    T12.convertTo(T12, CV_64F);
    T12.convertTo(T10, CV_64F);
    cv::Mat T20 = T10*T12.inv();
    std::cout << T20 << std::endl;
    std::map<std::string, cv::Mat> mat_map({{"external_transformation", T20}});
    Calibrator::dump_matrices(config_path2, mat_map);
    Calibrator::set_integrated(config_path2, true);
}
