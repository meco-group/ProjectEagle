#include "pattern.h"
#include <stdio.h>
#include <iostream>

using namespace eagle;

Pattern::Pattern(const cv::String& config, const type_t type) {
    switch (type) {
    case CALIBRATION: {
        cv::FileStorage fs(config, cv::FileStorage::READ);
        cv::String ttype = fs["calibrator"]["pattern"];
        fs.release();
        if (!ttype.compare("CHESSBOARD")) _pattern = new Chessboard(config);
        //if (!ttype.compare("CIRCLES_GRID")) _type = CIRCLES_GRID;
        //if (!ttype.compare("ASYMMETRIC_CIRCLES_GRID")) _type = ASYMMETRIC_CIRCLES_GRID;
        break;
    }

    case DETECTION: {
        _pattern = new CircleTriangle(config);
        break;
    }
    }
}

Pattern::Pattern(PatternInterface* pattern) :
    _pattern(pattern) {
    //do nothing else
}

Pattern::~Pattern() {
    //delete _pattern;
    //_pattern = NULL;
}

std::vector<cv::Point2f> Pattern::find(cv::Mat& img) {
    int t;
    return _pattern->find(img, t, false);
}

std::vector<cv::Point2f> Pattern::find(cv::Mat& img, int& id, bool draw) {
    return _pattern->find(img, id, draw);
}

std::vector<cv::Point3f> Pattern::reference() {
    return _pattern->reference();
}

std::vector<std::vector<cv::Point3f>> Pattern::reference(uint N) {
    std::vector<cv::Point3f> points = reference();
    return std::vector<std::vector<cv::Point3f>>(N, points);
}
