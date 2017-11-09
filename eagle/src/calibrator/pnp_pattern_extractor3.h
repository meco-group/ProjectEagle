#ifndef PNP_PATTERN_EXTRACTOR3_H
#define PNP_PATTERN_EXTRACTOR3_H

#include "pattern_extractor3.h"
#include <opencv/cv.hpp>

namespace eagle {

    class PnpPatternExtractor3 : public PatternExtractor3
    {
        private:
            cv::Mat _camera_matrix;
            cv::Mat _distortion_vector;

        public:
            PnpPatternExtractor3(const cv::String& config);
            PnpPatternExtractor3(const Pattern& pattern, const cv::Mat& camera_matrix, const cv::Mat& distortion_vector);

            virtual cloud3_t extract(const cv::Mat& img, bool display = false);

    };
};

#endif //PLANAR_PATTERN_EXTRACTOR3_H
