#ifndef CAMERA_INTERFACE_H
#define CAMERA_INTERFACE_H

#include <opencv2/core/core.hpp>

namespace eagle {

    class Camera {

        protected:
            cv::Mat _camera_matrix;
            cv::Mat _distortion_vector;

        public:
            Camera(int device) {
                _camera_matrix = cv::Mat::eye(3, 3, CV_32F);
                _distortion_vector = cv::Mat::zeros(5, 1, CV_32F);
            }

            void undistort(const cv::Mat& camera_matrix, const cv::Mat& distortion_vector) {
                _camera_matrix = camera_matrix;
                _distortion_vector = distortion_vector;
            }

            virtual bool start() = 0;
            virtual bool stop() = 0;
            virtual bool isOpened() = 0;
            virtual bool read(cv::Mat &img) = 0;
            virtual int getWidth() = 0;
            virtual int getHeight() = 0;
            virtual void setResolution(int width, int height) = 0;

    };

};

#endif //CAMERA_INTERFACE_H
