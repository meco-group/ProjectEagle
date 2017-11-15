#include "robot.h"

using namespace eagle;

Robot::Robot(uint id, double dx, double dy, const cv::Scalar& color) :
    _id(id), _dx(dx), _dy(dy), _color(color), _detected(false) {}

void Robot::update(const std::vector<cv::Point3f>& markers) {
    _markers = markers;
    compute_pose(markers);
    _detected = true;
}

void Robot::update(const eagle::marker_t& marker) {
    if (marker.id != _id) {
        return;
    }
    _markers.clear();
    _translation = cv::Point3f(marker.x, marker.y, marker.z);
    _rotation = cv::Point3f(marker.roll, marker.pitch, marker.yaw);
    _detected = true;
}

eagle::marker_t Robot::serialize() const {
    cv::Point3f t = translation();
    cv::Point3f r = rotation();

    eagle::marker_t m;
    m.id = _id;
    m.x = t.x;
    m.y = t.y;
    m.z = t.z;
    m.roll = r.x;
    m.pitch = r.y;
    m.yaw = r.z;
    return m;
}

void Robot::compute_pose(const std::vector<cv::Point3f>& markers) {
    _translation = ((1. / 2.) * (_markers[0] + _markers[1]));
    std::vector<cv::Point3f> unitvectors {ex(), ey(), ez()};
    cv::Mat R(unitvectors.size(), 3, CV_32FC1, unitvectors.data());
    R = R.t();
    _rotation = get_euler(R);
}

cv::Point3f Robot::translation() const {
    return _translation;
}

cv::Point3f Robot::rotation() const {
    return _rotation;
}

std::vector<cv::Point3f> Robot::vertices() const {
    std::vector<cv::Point3f> v; v.reserve(4);
    cv::Point3f i = ex();
    cv::Point3f j = ey();
    cv::Point3f t = translation();

    // make corners in local frame
    v.push_back((i * _dx + j * _dy) * 0.5);
    v.push_back((-i * _dx + j * _dy) * 0.5);
    v.push_back((-i * _dx - j * _dy) * 0.5);
    v.push_back((i * _dx - j * _dy) * 0.5);

    // move corners to global frame
    for (uint k = 0; k < v.size(); k++) {
        v[k] += t;
    }

    return v;
}

void Robot::draw_markers(cv::Mat& frame, Projection& projection) const {
    // markers
    if (_markers.size() == 3) {
        std::vector<cv::Point2f> markers_cam = projection.project_to_image(_markers);
        cv::circle(frame, markers_cam[0], 3, _color, -1);
        cv::circle(frame, markers_cam[1], 3, _color, -1);
        cv::circle(frame, markers_cam[2], 3, cv::Scalar(0, 0, 255), -1);
    }

    // Draw robot axes
    //cv::arrowedLine(frame, projection.project_to_image(translation()), projection.project_to_image(translation()+ex()), _color, 2);
    //cv::arrowedLine(frame, projection.project_to_image(translation()), projection.project_to_image(translation()+ey()), _color, 2);
}

void Robot::draw_id(cv::Mat& frame, Projection& projection) const {
    // id
    std::vector<cv::Point2f> markers_cam = projection.project_to_image(_markers);
    markers_cam.push_back(markers_cam[3]);
    for (uint k = 4; k < markers_cam.size(); k++) {
        cv::line(frame, markers_cam[k - 1], markers_cam[k], _color, 2, CV_AA);
    }
}

void Robot::draw_box(cv::Mat& frame, Projection& projection) const {
    // box
    std::vector<cv::Point2f> vertices_cam = projection.project_to_image(vertices());
    vertices_cam.push_back(vertices_cam[0]);
    for (uint i = 1; i < vertices_cam.size(); i++) {
        cv::line(frame, vertices_cam[i - 1], vertices_cam[i], _color, 2, CV_AA);
    }
}

void Robot::draw(cv::Mat& frame, Projection& projection) const {
    draw_markers(frame, projection);
    draw_id(frame, projection);
    draw_box(frame, projection);
}

cv::Point3f Robot::ex() const {
    cv::Point3f ex = _markers[2] - 0.5 * (_markers[0] + _markers[1]);
    ex /= cv::norm(ex);

    return ex;
}

cv::Point3f Robot::ey() const {
    cv::Point3f ey = ez().cross(ex());
    ey /= cv::norm(ey);
    return ey;
}

cv::Point3f Robot::ez() const {
    cv::Point3f ey_approx = _markers[0] - _markers[1];
    cv::Point3f ez = ex().cross(ey_approx);
    ez /= cv::norm(ez);

    return ez;
}

