#include <obstacle_rectangle.h>

using namespace eagle;

RectangleObstacle::RectangleObstacle(const cv::RotatedRect& rect, const cv::Mat& T) :
    Obstacle(T), _rect(rect)
{
    //do nothing
}

double RectangleObstacle::area() const {
    return _rect.size.area();
}

std::vector<cv::Point2f> RectangleObstacle::points2(uint) const {
    std::vector<cv::Point2f> pts(4);
    _rect.points(pts.data());

    return pts;
}

eagle::obstacle_t RectangleObstacle::serialize() const {
    std::vector<cv::Point2f> pts = points2();

    eagle::obstacle_t ret;
    ret.id = 0;
    ret.shape = eagle::RECTANGLE;
    ret.p1 = {pts[0].x, pts[0].y};
    ret.p2 = {pts[1].x, pts[1].y};
    ret.p3 = {pts[2].x, pts[2].y};

    return ret;
}
