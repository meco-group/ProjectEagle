#include "obstacle.h"
#include <math.h>

double Circle::area() const {
    return M_PI*pow(_radius, 2);
}

void Circle::draw(cv::Mat& frame, const cv::Matx23f& world2cam_tf) const {
    cv::Scalar gray(77, 76, 75);
    std::vector<cv::Point2f> position_cam;
    cv::transform(std::vector<cv::Point2f>{_position}, position_cam, world2cam_tf);
    cv::circle(frame, position_cam[0], _radius*world2cam_tf(0,0), gray, 2);
}

eagle::obstacle_t Circle::serialize() const {
    eagle::obstacle_t ret;
    ret.id = 0;
    ret.shape = eagle::CIRCLE;
    ret.p1 = {_position.x, _position.y};
    ret.p2 = {_position.x+_radius, _position.y};
    ret.p3 = {_position.x, _position.y+_radius};
    return ret;
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

void Rectangle::draw(cv::Mat& frame, const cv::Matx23f& world2cam_tf) const {
    cv::Scalar gray(77, 76, 75);
    std::vector<cv::Point2f> vertices_cam;
    cv::transform(_vertices, vertices_cam, world2cam_tf);
    int n = vertices_cam.size();
    for (uint i=0; i<n; i++) {
        cv::line(frame, vertices_cam[i], vertices_cam[(i+1)%n], gray, 2);
    }
}

eagle::obstacle_t Rectangle::serialize() const {
    eagle::obstacle_t ret;
    ret.id = 0;
    ret.shape = eagle::RECTANGLE;
    ret.p1 = {_position.x-0.5*_width*cos(_orientation)+0.5*_height*sin(_orientation), _position.y-0.5*_width*sin(_orientation)-0.5*_height*cos(_orientation)};
    ret.p2 = {_position.x+0.5*_width*cos(_orientation)+0.5*_height*sin(_orientation), _position.y+0.5*_width*sin(_orientation)-0.5*_height*cos(_orientation)};
    ret.p3 = {_position.x+0.5*_width*cos(_orientation)-0.5*_height*sin(_orientation), _position.y+0.5*_width*sin(_orientation)+0.5*_height*cos(_orientation)};
    return ret;
}