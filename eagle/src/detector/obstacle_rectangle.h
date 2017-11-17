#ifndef OBSTACLE_RECTANGLE_H
#define OBSTACLE_RECTANGLE_H

#include "obstacle.h"

namespace eagle {

class RectangleObstacle : public Obstacle {
  private:
    cv::RotatedRect _rect;

  public:
    RectangleObstacle(const cv::RotatedRect& rect);
    RectangleObstacle(const eagle::obstacle_t& obst);

    virtual std::string to_string() const;
    virtual double area() const;
    virtual cv::Point2f center() const;
    virtual double angle() const;
    virtual double width() const;
    virtual double height() const;
    virtual std::vector<cv::Point2f> points2(uint N = 100) const;

    virtual eagle::obstacle_t serialize() const;

};
};

#endif
