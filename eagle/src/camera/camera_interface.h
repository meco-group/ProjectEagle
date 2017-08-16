#ifndef CAMERA_INTERFACE_H
#define CAMERA_INTERFACE_H

#include <chrono>
#include <opencv2/core/core.hpp>

namespace eagle {
    class CameraInterface {
    protected:
        cv::Mat _camera_matrix;
        cv::Mat _distortion_vector;

    public:
        CameraInterface(int device);

        virtual bool isOpened() = 0;
        virtual bool read(cv::Mat &img) = 0;
        virtual double captureTime();

        virtual int getWidth() = 0;
        virtual int getHeight() = 0;
        void calibrate(const std::string& path);
    };
}

#endif //CAMERA_INTERFACE_H
