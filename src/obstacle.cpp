#include "obstacle.h"
#include <math.h>

double Circle::area() const {
    return M_PI*pow(_radius, 2);
}

void Circle::draw(cv::Mat& frame) const {
    cv::Scalar gray(77, 76, 75);
    cv::circle(frame, _position, _radius, gray, 2);
}

Rectangle::Rectangle(const cv::Point2f& position, double orientation, double width, double height) :
    Obstacle(position), _orientation(orientation), _width(width), _height(height) {
    pose2vertices(position, orientation, _vertices);
};

double Rectangle::area() const {
    return _width*_height;
}

void Rectangle::pose2vertices(const cv::Point2f& position, double orientation, std::vector<cv::Point2f>& vertices) const {
    cv::Point2f vert[4];
    cv::RotatedRect box(_position, cv::Size2f(_width, _height), (180./M_PI)*_orientation);
    box.points(vert);
    vertices = std::vector<cv::Point2f>(vert, vert+4);
}

void Rectangle::draw(cv::Mat& frame) const {
    cv::Scalar gray(77, 76, 75);
    int n = _vertices.size();
    for (uint i=0; i<n; i++) {
        cv::line(frame, _vertices[i], _vertices[(i+1)/n], gray, 2);
    }
}
