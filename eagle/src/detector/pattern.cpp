#include <pattern.h>
#include <stdio.h>
#include <iostream>

using namespace eagle;

Pattern::Pattern(const cv::String& config, const type_t type) {
    switch(type) {
        case CALIBRATION: {
            cv::FileStorage fs(config, cv::FileStorage::READ);
            cv::String ttype = fs["calibrator"]["pattern"];
            fs.release();
            if (!ttype.compare("CHESSBOARD")) _pattern = new Chessboard(config);
            //if (!ttype.compare("CIRCLES_GRID")) _type = CIRCLES_GRID;
            //if (!ttype.compare("ASYMMETRIC_CIRCLES_GRID")) _type = ASYMMETRIC_CIRCLES_GRID;
            break;}
    
        case DETECTION: {
            //_pattern = new CircleTriangle(config);
            break;}
    }
}

std::vector<cv::Point2f> Pattern::find(cv::Mat& img, bool draw)
{
    return _pattern->find(img, draw);
}

std::vector<cv::Point3f> Pattern::reference()
{
    return _pattern->reference();
}

std::vector<std::vector<cv::Point3f>> Pattern::reference(uint N) {
    std::vector<cv::Point3f> points = reference();
    return std::vector<std::vector<cv::Point3f>>(N, points);
}
