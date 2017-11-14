#ifndef PATTERN_H
#define PATTERN_H

#include <opencv/cv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "pattern_interface.h"
#include "chessboard.h"
#include "circle_triangle.h"

namespace eagle {

class Pattern {
  private:
    PatternInterface* _pattern = NULL;

  public:
    enum type_t {CALIBRATION, DETECTION};

    Pattern(const cv::String& config, const type_t type = CALIBRATION);
    Pattern(PatternInterface* pattern);
    ~Pattern();

    std::vector<cv::Point2f> find(cv::Mat& img);
    std::vector<cv::Point2f> find(cv::Mat& img, int& id, bool draw = false);
    std::vector<cv::Point3f> reference();
    std::vector<std::vector<cv::Point3f>> reference(uint N);

    PatternInterface* pattern() { return _pattern; }
};
};

#endif //PATTERN_H
