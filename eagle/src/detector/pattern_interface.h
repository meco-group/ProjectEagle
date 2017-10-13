#ifndef PATTERN_INTERFACE_H
#define PATTERN_INTERFACE_H

#include <opencv/cv.hpp>

namespace eagle {

    class PatternInterface
    {
        public:
            virtual std::vector<cv::Point2f> find(cv::Mat& img, bool draw = false) const = 0;
            virtual std::vector<cv::Point3f> reference() const = 0;
    };
};

#endif //PATTERN_INTERFACE_H
