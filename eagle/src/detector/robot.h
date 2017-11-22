#ifndef ROBOT_H
#define ROBOT_H

#include <iostream>
#include <stdint.h>
#include <math.h>
#include <string>
#include <vector>
#include "../utils/protocol.h"
#include "projection.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace eagle {

class Robot {

  private:
    unsigned int _id;
    double _dx;
    double _dy;
    bool _detected;
    cv::Scalar _color;
    std::vector<cv::Point3f> _markers;
    cv::Point3f _translation;
    cv::Point3f _rotation;

    cv::Point3f ex(const std::vector<cv::Point3f>& markers) const;
    cv::Point3f ey(const std::vector<cv::Point3f>& markers) const;
    cv::Point3f ez(const std::vector<cv::Point3f>& markers) const;
    void compute_pose(const std::vector<cv::Point3f>& markers);

  public:
    Robot(unsigned int id, double dx = 1.0, double dy = 1.0, const cv::Scalar& color = cv::Scalar(17, 110, 138));

    // setters
    void update(const cv::Point3f& translation, const cv::Point3f& rotation);
    void update(const std::vector<cv::Point3f>& markers);
    void update(const eagle::marker_t& marker);
    void reset() { _detected = false; }
    void draw(cv::Mat& frame, Projection& projection) const;
    void draw_markers(cv::Mat& frame, Projection& projection) const;
    void draw_id(cv::Mat& frame, Projection& projection) const;
    void draw_box(cv::Mat& frame, Projection& projection) const;

    // getters
    std::string to_string() const;
    unsigned int id() const { return _id; }
    unsigned int code() const { return id(); }
    cv::Scalar color() const { return _color; }
    std::vector<cv::Point3f> markers() const { return _markers; }
    bool detected() const { return _detected; }

    cv::Point3f translation() const;
    cv::Point3f rotation() const;
    std::vector<cv::Point3f> vertices() const;
    eagle::marker_t serialize() const;
};

};

#endif //ROBOT_H
