#include <chessboard.h>

using namespace eagle;

Chessboard::Chessboard(const int rows, const int cols, const double dimension) :
    CalibrationPattern(rows,cols,dimension)
{
    //do nothing
}

Chessboard::Chessboard(const cv::String& config) :
    CalibrationPattern(config)
{

}

std::vector<cv::Point2f> Chessboard::find(cv::Mat& img, bool draw) const
{
    std::vector<cv::Point2f> points;
    bool found = findChessboardCorners(img, size(), points, 0);

    if (found) {
        // improve the found coners' coordinate accuracy
        cv::Mat img_gray;
        cv::cvtColor(img, img_gray, cv::COLOR_BGR2GRAY);
        cv::cornerSubPix(img_gray, points, size(), cv::Size(-1,-1),
            cv::TermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1));
        // draw the corners
        if (draw) {
            cv::drawChessboardCorners(img, size(), cv::Mat(points), found);
        }
    } else {
        points.clear();
    }

    return points;
}
        
std::vector<cv::Point3f> Chessboard::reference() const
{
    std::vector<cv::Point3f> points;
    points.reserve(numel());

    for (int i=0; i<rows(); i++) {
        for (int j=0; j<cols(); j++) {
            points.push_back(cv::Point3d(j*dimension(), i*dimension(), 0));
        }
    }

    return points;
}
