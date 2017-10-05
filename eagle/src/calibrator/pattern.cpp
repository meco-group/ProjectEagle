#include <pattern.h>
#include <stdio.h>
#include <iostream>

using namespace eagle;

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

bool Pattern::find_pattern(cv::Mat& img, std::vector<cv::Point2f> &points, bool display)
{
    bool found = false;
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

    return found;
}

void Pattern::reference_pattern(std::vector<cv::Point3f> &points)
{
    // Reset the corners
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
}

