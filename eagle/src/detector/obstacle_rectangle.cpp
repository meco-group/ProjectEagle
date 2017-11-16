#include "obstacle_rectangle.h"

using namespace eagle;

RectangleObstacle::RectangleObstacle(const cv::RotatedRect& rect) : _rect(rect) {
    //do nothing
}

RectangleObstacle::RectangleObstacle(const eagle::obstacle_t& obst) {
    cv::Point2f center = cv::Point2f(0.5*(obst.p1.x+obst.p3.x), 0.5*(obst.p1.y+obst.p3.y));
    double width = sqrt(pow(obst.p2.x - obst.p3.x, 2) + pow(obst.p2.y - obst.p3.y, 2));
    double height = sqrt(pow(obst.p1.x - obst.p2.x, 2) + pow(obst.p1.y - obst.p2.y, 2));
    double angle = atan2(obst.p2.x-obst.p1.x, obst.p1.y-obst.p2.y);
    _rect = cv::RotatedRect(center, cv::Size2f(width, height), angle*180/M_PI);
}

double RectangleObstacle::area() const {
    return _rect.size.area();
}

cv::Point2f RectangleObstacle::center() const {
    return _rect.center;
}

double RectangleObstacle::angle() const {
    return _rect.angle * M_PI / 180;
}

double RectangleObstacle::width() const {
    return _rect.size.width;
}

double RectangleObstacle::height() const {
    return _rect.size.height;
}

std::vector<cv::Point2f> RectangleObstacle::points2(uint) const {
    std::vector<cv::Point2f> pts(4);
    _rect.points(pts.data());

    return pts;
}

eagle::obstacle_t RectangleObstacle::serialize() const {
    eagle::obstacle_t ret;
    ret.id = 0;
    ret.shape = eagle::RECTANGLE;
    cv::Point2f c = center();
    double w = width();
    double h = height();
    double ct = cos(angle());
    double st = sin(angle());
    // make sure points are always in fixed order
    cv::Point2f pt1 = cv::Point2f(c.x + 0.5 * w * ct - 0.5 * h * st, c.y + 0.5 * w * st + 0.5 * h * ct);
    cv::Point2f pt2 = cv::Point2f(c.x + 0.5 * w * ct + 0.5 * h * st, c.y + 0.5 * w * st - 0.5 * h * ct);
    cv::Point2f pt3 = cv::Point2f(c.x - 0.5 * w * ct + 0.5 * h * st, c.y - 0.5 * w * st - 0.5 * h * ct);

    ret.p1 = {pt1.x, pt1.y};
    ret.p2 = {pt2.x, pt2.y};
    ret.p3 = {pt3.x, pt3.y};
    return ret;
}
