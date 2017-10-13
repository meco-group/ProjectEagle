#include <calibration_pattern.h>

using namespace eagle;

CalibrationPattern::CalibrationPattern(const int rows, const int cols, const double dimension) :
    _rows(rows), _cols(cols), _dimension(dimension)
{

}

CalibrationPattern::CalibrationPattern(const cv::String& config) {
    // parse config file
    cv::FileStorage fs(config, cv::FileStorage::READ);
    _rows = fs["calibrator"]["board_height"];
    _cols = fs["calibrator"]["board_width"];
    _dimension = fs["calibrator"]["square_size"];
    fs.release();
}
