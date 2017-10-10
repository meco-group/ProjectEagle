#include <pattern.h>
#include <stdio.h>
#include <iostream>

using namespace eagle;

Pattern::Pattern(const type_t type, const int rows, const int cols, const double dimension) : 
    _type(type), _rows(rows), _cols(cols), _dimension(dimension)
{
    //do nothing
}

Pattern::Pattern(const cv::String& config) {
    // parse config file
    cv::FileStorage fs(config, cv::FileStorage::READ);
    _rows = fs["calibrator"]["board_height"];
    _cols = fs["calibrator"]["board_width"];
    _dimension = fs["calibrator"]["square_size"];
    cv::String ttype = fs["calibrator"]["pattern"];
    fs.release();

    // set pattern type right
    _type = INVALID;
    if (!ttype.compare("CHESSBOARD")) _type = CHESSBOARD;
    if (!ttype.compare("CIRCLES_GRID")) _type = CIRCLES_GRID;
    if (!ttype.compare("ASYMMETRIC_CIRCLES_GRID")) _type = ASYMMETRIC_CIRCLES_GRID;
}

Pattern Pattern::Chessboard(const int rows, const int cols, const double dimension)
{
    return Pattern(CHESSBOARD, rows, cols, dimension);
}

Pattern Pattern::Circles(const int rows, const int cols, const double dimension)
{
    return Pattern(CIRCLES_GRID, rows, cols, dimension);
}

Pattern Pattern::AsymCircles(const int rows, const int cols, const double dimension)
{
    return Pattern(ASYMMETRIC_CIRCLES_GRID, rows, cols, dimension);
}

std::vector<cv::Point2f> Pattern::find(const cv::Mat& img, bool display)
{
    bool found = false;
    std::vector<cv::Point2f> points;

    switch (_type) {
        case CHESSBOARD:
            found = findChessboardCorners(img, size(), points, 0);
            if (found) {
                // improve the found coners' coordinate accuracy
                cv::Mat img_gray;
                cv::cvtColor(img, img_gray, cv::COLOR_BGR2GRAY);
                cv::cornerSubPix(img_gray, points, size(), cv::Size(-1,-1),
                    cv::TermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1));
                // draw the corners
                cv::drawChessboardCorners(img, size(), cv::Mat(points), found);
            }
            break;
        case CIRCLES_GRID:
            found = findCirclesGrid(img, size(), points);
            break;
        case ASYMMETRIC_CIRCLES_GRID:
            found = findCirclesGrid(img, size(), points, cv::CALIB_CB_ASYMMETRIC_GRID);
            break;
    }
    
    if(display) {
        imshow("Image", img);
        cv::waitKey(500);
    }

    if (!found){
        points.clear();
    }

    return points;
}

std::vector<cv::Point3f> Pattern::reference()
{
    // Reset the corners
    std::vector<cv::Point3f> points;
    points.clear();

    // Calculate grid points based on pattern
    switch(_type) {
        case CHESSBOARD:
            //pass through
        case CIRCLES_GRID:
            for (int i=0; i<_rows; i++) {
                for (int j=0; j<_cols; j++) {
                    points.push_back(cv::Point3f(j*_dimension, i*_dimension, 0));
                }
            }
            break;
        case ASYMMETRIC_CIRCLES_GRID:
            for (int i=0; i<_rows; i++) {
                for (int j = 0; j<_cols; j++) {
                    points.push_back(cv::Point3f((2*j+i%2)*_dimension, i*_dimension, 0));
                }
            }
            break;
    }

    return points;
}

std::vector<std::vector<cv::Point3f>> Pattern::reference(uint N) {
    std::vector<cv::Point3f> points = reference();
    return std::vector<std::vector<cv::Point3f>>(N, points);
}
