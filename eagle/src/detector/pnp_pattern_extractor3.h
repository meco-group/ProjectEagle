#ifndef PNP_PATTERN_EXTRACTOR3_H
#define PNP_PATTERN_EXTRACTOR3_H

#include "pattern_extractor3.h"
#include "transform.h"
#include <opencv/cv.hpp>

namespace eagle {

class PnpPatternExtractor3 : public PatternExtractor3 {
  private:
    cv::Mat _camera_matrix;
    cv::Mat _distortion_vector;
    cv::Mat _T;

  public:
    PnpPatternExtractor3(const cv::String& config);
    PnpPatternExtractor3(const Pattern& pattern, const cv::Mat& camera_matrix, const cv::Mat& distortion_vector, const cv::Mat& T);

    virtual cloud3_t extract(cv::Mat& img, const cv::Point2f& offset, int& id, bool display = false);
    virtual void set_transform(const cv::Mat& T);

};
};

#endif //PLANAR_PATTERN_EXTRACTOR3_H
