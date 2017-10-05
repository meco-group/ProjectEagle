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
    
            Pattern(const type_t type = INVALID, const int rows = 0, const int cols = 0, const double dimension = 1.0) : 
                _type(type), _rows(rows), _cols(cols), _dimension(dimension) {}
            static Pattern Chessboard(const int rows, const int cols, const double dimension = 1.0);
            static Pattern Circles(const int rows, const int cols, const double dimension = 1.0);
            static Pattern AsymCircles(const int rows, const int cols, const double dimension = 1.0);
    
            const int rows() { return _rows; }
            const int cols() { return _cols; }
            const cv::Size size(){ return cv::Size(_cols, _rows); } //_rows, _cols); }
            const int numel() { return _rows*_cols; }
            const double dimension() { return _dimension; }
    
            bool find_pattern(cv::Mat& img, std::vector<cv::Point2f> &points, bool display = true);
            void reference_pattern(std::vector<cv::Point3f> &points);
    
    };
};

#endif //PATTERN_H
