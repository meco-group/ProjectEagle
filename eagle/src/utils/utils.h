#ifndef UTILS_H
#define UTILS_H

#define CONFIG_PATH "/home/odroid/ProjectEagle/eagle/config/config.xml"
#define CAL_IMAGES_PATH "/home/odroid/ProjectEagle/eagle/config/calibration/"

#include <cstdio>
#include <zconf.h>
#include <chrono>
#include <iostream>
#include <opencv2/core/core.hpp>

namespace eagle {

    int kbhit() {
        struct timeval tv;
        fd_set fds;
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
        select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
        return FD_ISSET(STDIN_FILENO, &fds);
    }

    unsigned long timestamp() {
        return std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    cv::Matx33f image2ground_tf(const cv::Mat& ground_plane, const cv::Mat& camera_matrix, const cv::Mat& l_tf) {
        cv::Mat gp, cm, ext_tf;
        ground_plane.convertTo(gp, CV_32F);
        camera_matrix.convertTo(cm, CV_32F);
        l_tf.convertTo(ext_tf, CV_32F);
        cv::Mat t1, t2, int_tf, sel_tf;
        float h = -gp.at<float>(0,3)/gp.at<float>(0,2);
        cv::hconcat(cv::Mat::zeros(1, 2, CV_32F), cv::Mat::ones(1, 1, CV_32F), t2);
        cv::vconcat(h*cm.inv(), t2, int_tf);

        cv::hconcat(cv::Mat::eye(2, 2, CV_32F), cv::Mat::zeros(2, 2, CV_32F), t1);
        cv::hconcat(cv::Mat::zeros(1, 3, CV_32F), cv::Mat::ones(1, 1, CV_32F), t2);
        cv::vconcat(t1, t2, sel_tf);

        cv::Mat tf = sel_tf*ext_tf*int_tf;
        return cv::Matx33f((float*)(tf.ptr()));
    }

}

#endif //UTILS_H

