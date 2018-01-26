#include <eagle.h>
#include <iostream>
#include <fstream>
#include <map>
#include <opencv/cv.hpp>

using namespace eagle;

/*
 * Compute the internal calibration matrix together with the distortion vector.
 * These are saved in the provided config file.
 */

int main(int argc, char* argv[]) {
    std::string config_path = (argc > 1) ? argv[1] : CONFIG_PATH;
    std::string images_path = (argc > 2) ? argv[2] : CAL_IMAGES_PATH;

    // extract pattern from images
    PatternExtractor extractor(config_path);
    std::vector<cloud2_t> image_pnts = extractor.extract(images_path, false);
    std::vector<std::vector<cv::Point3f>> pattern_pnts = extractor.pattern().reference(image_pnts.size());

    // Read config file
    cv::FileStorage fs(config_path, cv::FileStorage::READ);
    int flag = 0;
    if (((int)fs["calibrator"]["calib_fix_principal_point"] == 1)) flag |= CV_CALIB_FIX_PRINCIPAL_POINT;
    if (((int)fs["calibrator"]["calib_zero_tangent_dist"] == 1)) flag |= CV_CALIB_ZERO_TANGENT_DIST;
    if (((int)fs["calibrator"]["calib_fix_aspect_ratio"] == 1)) flag |= CV_CALIB_FIX_ASPECT_RATIO;
    int width = fs["camera"]["resolution"]["width"];
    int height = fs["camera"]["resolution"]["height"];
    cv::Size size(width,height);
    fs.release();

    // initialize camera_matrix & distortion_vector
    cv::Mat camera_matrix = cv::Mat::eye(3, 3, CV_64F);
    if (flag & CV_CALIB_FIX_ASPECT_RATIO) {
        camera_matrix.at<double>(0, 0) = 1.0;
    }
    cv::Mat distortion_vector = cv::Mat::zeros(8, 1, CV_64F);

    // find intrinsic and extrinsic camera parameters
    cv::Mat rvecs, tvecs;
    double reproj_error = cv::calibrateCamera(pattern_pnts, image_pnts, size, camera_matrix,
        distortion_vector, rvecs, tvecs, flag|CV_CALIB_FIX_K4|CV_CALIB_FIX_K5);
    std::cout << "Calibration rms error: " << reproj_error << std::endl;
    if(reproj_error > 2) {
        std::cout << "Reprojection error is too high to consider the camera calibration succesful.." << std::endl << "Calibration failed." << std::endl;
        return -1;
    }
    if(!(cv::checkRange(camera_matrix) && cv::checkRange(distortion_vector))){
        std::cout << "The camera_matrix or distortion vector were out of range." << std::endl << "Calibration failed." << std::endl;
        return -1;
    }

    // print the parameters
    std::cout << "camera matrix " << camera_matrix << std::endl;
    std::cout << "distortion vector " << distortion_vector << std::endl;
    // save parameters
    std::map<std::string, cv::Mat> matrices;
    matrices["camera_matrix"] = camera_matrix;
    matrices["distortion_vector"] = distortion_vector;
    dump_matrices(config_path, matrices);
    set_calibrated(config_path, true);
    std::cout << "Saved calibration information succesfully." << std::endl;
};
