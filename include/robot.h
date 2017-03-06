#ifndef ROBOT_H
#define ROBOT_H


#include <iostream>
#include <stdint.h>
#include <math.h>
#include <string>
#include <vector>
#include "protocol.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

class Robot {

    private:
        uint _code;
        double _width;
        double _height;
        cv::Point2f _position;
        double _orientation;
        std::vector<cv::Point2f> _markers;
        std::vector<cv::Point2f> _vertices;
        bool _detected;
        cv::Scalar _color;

        void markers2pose(const std::vector<cv::Point2f>& markers, cv::Point2f& position, double& orientation) const;
        void pose2vertices(const cv::Point2f& position, double orientation, std::vector<cv::Point2f>& vertices) const;

    public:
        Robot(uint code, double width, double height);
        Robot(uint code, double width, double height, const cv::Scalar& color);
        void update(const std::vector<cv::Point2f>& markers);
        bool detected() const;
        uint code() const;
        void reset();
        std::vector<cv::Point2f> vertices() const;
        void draw(cv::Mat& frame, const cv::Matx23f& world2cam_tf) const;
        eagle::marker_t serialize() const;
};

#endif //ROBOT_H
