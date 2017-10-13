#ifndef PATTERN_H
#define PATTERN_H

#include <opencv/cv.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace eagle {

    class Pattern
    {
        private:
            enum type_t { INVALID, CHESSBOARD, CIRCLES_GRID, ASYMMETRIC_CIRCLES_GRID };
    
            type_t _type;
            int _rows;
            int _cols;
            double _dimension;
    
        public:
            Pattern(const type_t type = INVALID, const int rows = 0, const int cols = 0, const double dimension = 1.0);
            Pattern(const cv::String& config);
            static Pattern Chessboard(const int rows, const int cols, const double dimension = 1.0);
            static Pattern Circles(const int rows, const int cols, const double dimension = 1.0);
            static Pattern AsymCircles(const int rows, const int cols, const double dimension = 1.0);
    
            int rows() { return _rows; }
            int cols() { return _cols; }
            const cv::Size size(){ return cv::Size(_cols, _rows); }
            int numel() { return _rows*_cols; }
            double dimension() { return _dimension; }
    
            std::vector<cv::Point2f> find(const cv::Mat& img, bool display = false);
            std::vector<cv::Point3f> reference();
            std::vector<std::vector<cv::Point3f>> reference(uint N);
    
    };
};

#endif //PATTERN_H
