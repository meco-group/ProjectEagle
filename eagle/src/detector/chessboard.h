#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include "calibration_pattern.h"

namespace eagle {

class Chessboard : public CalibrationPattern {
  public:
    Chessboard(const int rows, const int cols, const double dimension = 1.0);
    Chessboard(const cv::String& config);

    virtual std::vector<cv::Point2f> find(cv::Mat& img) const;
    virtual std::vector<cv::Point2f> find(cv::Mat& img, int& id, bool draw = false) const;
    virtual std::vector<cv::Point3f> reference() const;

};
};

#endif //CHESSBOARD_H
