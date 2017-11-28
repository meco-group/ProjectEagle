#include "obstacle.h"
#include "obstacle_rectangle.h"
#include "obstacle_circle.h"

using namespace eagle;

Obstacle::Obstacle() {
}

std::vector<cv::Point3f> Obstacle::points(uint N) const {
    return addz(points2(N), 0.0f);
}

void Obstacle::draw(cv::Mat& frame, Projection& projection) const {
    std::vector<cv::Point2f> p = projection.project_to_image(points());
    p.push_back(p[0]);

    cv::Scalar gray(77, 76, 75);
    for (uint i = 1; i < p.size(); i++) {
        cv::line(frame, p[i], p[i - 1], gray, 2, CV_AA);
    }
}

Obstacle* Obstacle::deserialize(const eagle::obstacle_t& obst) {
    switch (obst.shape) {
    case RECTANGLE:
        return new RectangleObstacle(obst);
        break;
    case CIRCLE:
        return new CircleObstacle(obst);
        break;
    default:
        std::cout << "Obstacle shape not understood." << std::endl;
        return NULL;
    }
}
