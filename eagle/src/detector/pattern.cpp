#include "pattern.h"
#include <stdio.h>
#include <iostream>
#include "../utils/rapidxml.hpp"

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

std::vector<Pattern*> Pattern::get_pattern_list(const std::string& marker_config) {
    using namespace rapidxml;
    std::ifstream file(marker_config);
    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    buffer.push_back('\0');
    file.close();
    std::vector<Pattern*> patterns;
    xml_document<> doc;
    doc.parse<0>(&buffer[0]);
    xml_node<>* ct_node = doc.first_node("circle_triangle");
    xml_node<>* instance;
    if (ct_node != NULL) {
        instance = ct_node->first_node("instance");
        while (instance != NULL) {
            std::cout << "Parsing " << instance->first_node("name")->value() << std::endl;
            int id = std::stoi(instance->first_node("id")->value());
            xml_node<>* dim = instance->first_node("dimensions");
            cv::Point2f dimension(std::stod(dim->first_node("x")->value()), std::stod(dim->first_node("y")->value()));
            cv::Point2f qr_size(std::stod(dim->first_node("qrx")->value()), std::stod(dim->first_node("qry")->value()));
            patterns.push_back(new Pattern(new CircleTriangle(dimension, qr_size)));
            instance = instance->next_sibling("instance");
        }

    }

    return patterns;

}
