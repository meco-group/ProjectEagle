#include "obstacle_circle.h"

using namespace eagle;

CircleObstacle::CircleObstacle(const cv::Point2f& center, const float radius) :
    _center(center), _radius(radius) {
    //do nothing
}

CircleObstacle::CircleObstacle(const eagle::obstacle_t& obst) :
    CircleObstacle(cv::Point2f(obst.p1.x, obst.p1.y), obst.p2.x - obst.p2.y) {
}

std::string CircleObstacle::to_string() const {
    cv::Point2f c = center();
    char s[100];
    sprintf(s, "Circular obstacle at (%.1f,%.1f) with radius %.1f.", c.x, c.y, radius());
    return std::string(s);
}

double CircleObstacle::area() const {
    return M_PI * _radius * _radius;
}

cv::Point2f CircleObstacle::center() const {
    return _center;
}

double CircleObstacle::radius() const {
    return _radius;
}

std::vector<cv::Point2f> CircleObstacle::points2(uint N) const {
    std::vector<cv::Point2f> points(N);
    for (uint k = 0; k < N; k++) {
        float t = k * M_PI * 2.0 / N;
        points[k] = _center + _radius * cv::Point2f(cosf(t), sinf(t));
    }

    return points;
}

eagle::obstacle_t CircleObstacle::serialize() const {
    eagle::obstacle_t ret;
    ret.id = 0;
    ret.shape = eagle::CIRCLE;
    ret.p1 = {_center.x, _center.y};
    ret.p2 = {_center.x + _radius, _center.y};
    ret.p3 = {_center.x, _center.y + _radius};
    return ret;

}
