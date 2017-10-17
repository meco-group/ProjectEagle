#ifndef EAGLE_TRANSFORM_H
#define EAGLE_TRANSFORM_H

#include <iostream>
#include <fstream>
#include <opencv/cv.hpp>

namespace eagle {

    cv::Mat compute_transform(const cv::Mat& points1, const cv::Mat& points2);
    cv::Mat compute_transform(std::vector<cv::Point3f>& points1, std::vector<cv::Point3f>& points2);
    cv::Mat compute_transform(std::vector<std::vector<cv::Point3f>>& points1, std::vector<std::vector<cv::Point3f>>& points2);

    cv::Mat compute_inplane_transform(const cv::Point3f& n1);
    cv::Mat compute_inplane_transform(const cv::Mat& plane);

    cv::Mat get_rotation(const cv::Mat& T);
    cv::Point3f get_euler(const cv::Mat& T);
    cv::Mat get_translation(const cv::Mat& T);
    
    cv::Point3f transform(const cv::Mat& T, const cv::Point3f& p);
    std::vector<cv::Point3f> transform(const cv::Mat& T, const std::vector<cv::Point3f>& points);
    std::vector<std::vector<cv::Point3f>> transform(const cv::Mat& T, const std::vector<std::vector<cv::Point3f>>& points);

    cv::Point2f transform2(const cv::Mat& T, const cv::Point3f& p);
    std::vector<cv::Point2f> transform2(const cv::Mat& T, const std::vector<cv::Point3f>& points);
    std::vector<std::vector<cv::Point2f>> transform2(const cv::Mat& T, const std::vector<std::vector<cv::Point3f>>& points);

    cv::Point2f dropz(const cv::Point3f& p);
    std::vector<cv::Point2f> dropz(const std::vector<cv::Point3f>& points);
    std::vector<std::vector<cv::Point2f>> dropz(const std::vector<std::vector<cv::Point3f>>& points);

    cv::Point3f addz(const cv::Point2f& p, float z = 0.0f);
    std::vector<cv::Point3f> addz(const std::vector<cv::Point2f>& points, float z = 0.0f);
    std::vector<std::vector<cv::Point3f>> addz(const std::vector<std::vector<cv::Point2f>>& points, float z = 0.0f);

    cv::Mat transform_plane(const cv::Mat& T, const cv::Mat& plane);

};

#endif // EAGLE_TRANSFORM_H
