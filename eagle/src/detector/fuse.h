#ifndef FUSE_H
#define FUSE_H

#include "projection.h"

namespace eagle {

void remapinf(std::string config, const cv::Mat& img, cv::Mat& warped, const float k, const cv::Size& siz, double z0 = 0);
void remapinf_cropped(std::string config, const cv::Mat& img, cv::Mat& warped, const float k, cv::Point2f& offset, double z0 = 0);
void remapinf_canvas(Projection& projection, const cv::Mat& img, cv::Mat& warped, const float k, cv::Point2f& offset, cv::Size& siz, double z0 = 0);
void overlay(const cv::Mat& in1, const cv::Mat& in2, cv::Mat& out);
void replace(const cv::Mat& in, cv::Mat& out, double weight = 0.5);

};

#endif //FUSE_H
