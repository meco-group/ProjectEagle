#include "robot.h"

using namespace eagle;

Robot::Robot(unsigned int id, double dx, double dy, const cv::Scalar& color) :
    Robot(id, dx, dy, cv::Point3f(0, 0, 0), color) {}

Robot::Robot(unsigned int id, double dx, double dy, const cv::Point3f& marker_translation, const cv::Scalar& color) :
    Robot(id, dx, dy, marker_translation, cv::Point3f(0, 0, 0), color) {}

Robot::Robot(unsigned int id, double dx, double dy, const cv::Point3f& marker_translation, const cv::Point3f& marker_rotation, const cv::Scalar& color) :
    _id(id), _dx(dx), _dy(dy), _marker_translation(marker_translation), _marker_rotation(marker_rotation), _color(color), _detected(false), _is_obstacle(false) {}

void Robot::update(const std::vector<cv::Point3f>& markers) {
    _markers = markers;
    compute_pose(markers);
    _detected = true;
}

void Robot::update(const cv::Point3f& translation, const cv::Point3f& rotation) {
    _markers.clear();
    _translation = translation;
    _rotation = rotation;
    _detected = true;
}

void Robot::update(const eagle::marker_t& marker) {
    if (marker.id != _id) {
        return;
    }
    update(cv::Point3f(marker.x, marker.y, marker.z), cv::Point3f(marker.roll, marker.pitch, marker.yaw));
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
    std::vector<cv::Point3f> unitvectors {ex(markers), ey(markers), ez(markers)};
    cv::Mat R(unitvectors.size(), 3, CV_32FC1, unitvectors.data());
    cv::Mat R2;
    euler_to_R(_marker_rotation).convertTo(R2, CV_32F);
    R = R.t()*R2.t();
    _rotation = get_euler(R);
    _translation = ((1. / 2.) * (markers[0] + markers[1])) + cv::Point3f(cv::Mat(R*cv::Mat(-_marker_translation)));
}

std::string Robot::to_string() const {
    cv::Point3f t = translation();
    cv::Point3f r = rotation();
    char s[100];
    sprintf(s, "Robot [%d] with position (%.1f,%.1f,%.1f) and rotation (%.1f,%.1f,%.1f).", id(), t.x, t.y, t.z, r.x, r.y, r.z);
    return std::string(s);
}

cv::Point3f Robot::translation() const {
    return _translation;
}

cv::Point3f Robot::rotation() const {
    return _rotation;
}

std::vector<cv::Point3f> Robot::vertices() const {
    std::vector<cv::Point3f> v; v.reserve(4);
    cv::Mat R = euler_to_R(_rotation);
    cv::Point3f i = cv::Point3f(R.col(0));
    cv::Point3f j = cv::Point3f(R.col(1));
    cv::Point3f t = translation();
    // make corners in local frame
    v.push_back((i * _dx + j * _dy) * 0.5);
    v.push_back((-i * _dx + j * _dy) * 0.5);
    v.push_back((-i * _dx - j * _dy) * 0.5);
    v.push_back((i * _dx - j * _dy) * 0.5);

    // move corners to global frame
    for (unsigned int k = 0; k < v.size(); k++) {
        v[k] += t;
    }
    return v;
}

void Robot::draw_markers(cv::Mat& frame, Projection& projection) const {
    // markers
    if (_markers.size() >= 3) {
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
    if (_markers.size() >= 7) {
        std::vector<cv::Point2f> markers_cam = projection.project_to_image(_markers);
        markers_cam.push_back(markers_cam[3]);
        for (unsigned int k = 4; k < markers_cam.size(); k++) {
            cv::line(frame, markers_cam[k - 1], markers_cam[k], _color, 2, CV_AA);
        }
    }
}

void Robot::draw_box(cv::Mat& frame, Projection& projection) const {
    // box
    std::vector<cv::Point2f> vertices_cam = projection.project_to_image(vertices());
    vertices_cam.push_back(vertices_cam[0]);
    for (unsigned int i = 1; i < vertices_cam.size(); i++) {
        cv::line(frame, vertices_cam[i - 1], vertices_cam[i], _color, 2, CV_AA);
    }
}

void Robot::draw(cv::Mat& frame, Projection& projection) const {
    draw_markers(frame, projection);
    // draw_id(frame, projection);
    draw_box(frame, projection);
}

cv::Point3f Robot::ex(const std::vector<cv::Point3f>& markers) const {
    cv::Point3f ex = markers[2] - 0.5 * (markers[0] + markers[1]);
    ex /= cv::norm(ex);

    return ex;
}

cv::Point3f Robot::ey(const std::vector<cv::Point3f>& markers) const {
    cv::Point3f ey = ez(markers).cross(ex(markers));
    ey /= cv::norm(ey);
    return ey;
}

cv::Point3f Robot::ez(const std::vector<cv::Point3f>& markers) const {
    cv::Point3f ey_approx = markers[0] - markers[1];
    cv::Point3f ez = ex(markers).cross(ey_approx);
    ez /= cv::norm(ez);

    return ez;
}

Obstacle* Robot::to_obstacle() const {
    cv::Point2f center(_translation.x, _translation.y);
    cv::Point2f size(_dx, _dy);
    double angle = _rotation.z;
    return new RectangleObstacle(center, size, angle);
}
