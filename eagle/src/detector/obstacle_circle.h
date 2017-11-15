#ifndef OBSTACLE_CIRCLE_H
#define OBSTACLE_CIRCLE_H

#include "obstacle.h"

namespace eagle {

class CircleObstacle : public Obstacle {
  private:
    cv::Point2f _center;
    float _radius;

  public:
    CircleObstacle(const cv::Point2f& center, const float radius, const cv::Mat& T);

    virtual double area() const;
    virtual std::vector<cv::Point2f> points2(uint N = 100) const;

    virtual eagle::obstacle_t serialize() const;

};
};

#endif