#ifndef CALIBRATION_PATTERN_H
#define CALIBRATION_PATTERN_H

#include <pattern_interface.h>

namespace eagle {

    class CalibrationPattern : public PatternInterface
    {
        private:
            int _rows;
            int _cols;
            double _dimension;
    
        public:
            CalibrationPattern(const int rows = 1, const int cols = 1, const double dimension = 1.0);
            CalibrationPattern(const cv::String& config);
    
            int rows() const { return _rows; }
            int cols() const { return _cols; }
            const cv::Size size() const { return cv::Size(_cols, _rows); }
            int numel() const { return _rows*_cols; }
            double dimension() const { return _dimension; }
    
            virtual std::vector<cv::Point2f> find(cv::Mat& img, bool draw = false) const = 0;
            virtual std::vector<cv::Point3f> reference() const = 0;
    
    };
};

#endif
