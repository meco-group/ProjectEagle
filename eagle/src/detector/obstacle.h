#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "protocol.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <projection.h>

namespace eagle {

    class Obstacle {
        private:
            cv::Mat _T;

        public:
            Obstacle(const cv::Mat& T);

            std::vector<cv::Point3f> points(uint N = 100) const;
            void draw(cv::Mat& frame, Projection& projection) const;

            virtual double area() const = 0;
            virtual std::vector<cv::Point2f> points2(uint N = 100) const = 0;
            virtual eagle::obstacle_t serialize() const = 0;
    };
};

#endif //OBSTACLE_H
