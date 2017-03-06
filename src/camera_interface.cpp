#include "camera_interface.h"

CameraInterface::CameraInterface(int device) {
    _camera_matrix = cv::Mat::eye(3, 3, CV_32F);
    _distortion_vector = cv::Mat::zeros(5, 1, CV_32F);
}


double CameraInterface::captureTime()
{
    uint32_t ms;
    double s;
    ms = std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch()).count();
    s = ms * double(std::chrono::milliseconds::period::num) / std::chrono::milliseconds::period::den;

    return s;
}

void CameraInterface::calibrate(const std::string& path) {
    cv::FileStorage fs(path, cv::FileStorage::READ);
    fs["camera_matrix"] >> _camera_matrix;
    fs["distortion_vector"] >> _distortion_vector;
}
