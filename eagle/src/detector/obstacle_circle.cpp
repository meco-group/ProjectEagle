#include <obstacle_circle.h>

using namespace eagle;

CircleObstacle::CircleObstacle(const cv::Point2f& center, const float radius, const cv::Mat& T) : 
    Obstacle(T), _center(center), _radius(radius)
{
    //do nothing
}

double CircleObstacle::area() const {
    return M_PI*_radius*_radius;
}

std::vector<cv::Point2f> CircleObstacle::points2(uint N) const {
    std::vector<cv::Point2f> points(N);
    for (uint k=0; k<N; k++) {
        float t = k*M_PI*2.0/N;
        points[k] = _center + _radius*cv::Point2f(cosf(t), sinf(t));
    }

    return points;
}

eagle::obstacle_t CircleObstacle::serialize() const {
    eagle::obstacle_t ret;
    ret.id = 0;
    ret.shape = eagle::CIRCLE;
    ret.p1 = {_center.x, _center.y};
    ret.p2 = {_center.x+_radius, _center.y};
    ret.p3 = {_center.x, _center.y+_radius};
    return ret;

}
