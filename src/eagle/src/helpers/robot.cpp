#include "robot.h"

Robot::Robot(uint code, double width, double height, const cv::Scalar& color, cv::Mat T, cv::Mat R) :
        _code(code), _width(width), _height(height), _color(color), _detected(false), _T(T), _R(R) {}

Robot::Robot(uint code, double width, double height, cv::Mat T, cv::Mat R) {
    Robot(code, width, height, cv::Scalar(17, 110, 138), R, T);
}

Robot::Robot(uint code, double width, double height) {
    Robot(code, width, height, cv::Scalar(17, 110, 138));
}

Robot::Robot(uint code, double width, double height, const cv::Scalar& color) {
    cv::Mat R = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat T = cv::Mat::zeros(1, 3, CV_64F);
    Robot(code, width, height, color, R, T);
}


void Robot::update(const std::vector<cv::Point2f>& markers) {
    _markers = markers;
    _detected = true;
    markers2pose(markers, _position, _orientation);
    pose2vertices(_position, _orientation, _vertices);
    pose2global(_position, _orientation);
}

bool Robot::detected() const {
    return _detected;
}

uint Robot::code() const {
    return _code;
}

void Robot::reset() {
    _detected = false;
}

void Robot::markers2pose(const std::vector<cv::Point2f>& markers, cv::Point2f& position, double& orientation) const {
    // markers: left - right - top
    position = (1./3.)*(markers[0]+markers[1]+markers[2]);
    orientation = atan2((markers[2].y-position.y), (markers[2].x-position.x));
}

void Robot::pose2vertices(const cv::Point2f& position, double orientation, std::vector<cv::Point2f>& vertices) const {
    cv::Point2f vert[4];
    cv::RotatedRect box(_position, cv::Size2f(_width, _height), (180./M_PI)*_orientation);
    box.points(vert);
    vertices = std::vector<cv::Point2f>(vert, vert+4);
}

void Robot::pose2global(cv::Point2f& position, double& orientation) const {
    std::cout << _T << "\n" << cv::Point2f(_T.at<float>(0,0), _T.at<float>(1,0)) << "\n";
    position = position + cv::Point2f(_T.at<float>(0,0), _T.at<float>(1,0));
}

std::vector<cv::Point2f> Robot::vertices() const {
    return _vertices;
}

void Robot::draw(cv::Mat& frame, const cv::Matx23f& world2cam_tf) const {
    std::vector<cv::Point2f> markers_cam;
    cv::transform(_markers, markers_cam, world2cam_tf);
    // markers
    cv::circle(frame, markers_cam[0], 3, _color, -1);
    cv::circle(frame, markers_cam[1], 3, _color, -1);
    cv::circle(frame, markers_cam[2], 3, cv::Scalar(0, 0, 255), -1);
    // box
    // std::vector<cv::Point2f> vertices_cam;
    // cv::transform(_vertices, vertices_cam, world2cam_tf);
    // int n = vertices_cam.size();
    // for (uint i=0; i<n; i++) {
    //     cv::line(frame, vertices_cam[i], vertices_cam[(i+1)%n], _color, 2);
    // }
}

eagle::marker_t Robot::serialize() const {
    return {(int)_code, _position.x, _position.y, _orientation};
}
