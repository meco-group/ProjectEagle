#ifndef FUSE_H
#define FUSE_H

#include <projection.h>

namespace eagle {

    void remapinf(std::string config, const cv::Mat& img, cv::Mat& warped, uint r, const cv::Size& siz, double z0 = 0);
    void overlay(const cv::Mat& in1, const cv::Mat& in2, cv::Mat& out);
    void replace(const cv::Mat& in, cv::Mat& out, double weight = 0.5);

};

#endif //FUSE_H
