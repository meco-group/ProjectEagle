#include "pattern_extractor3.h"

using namespace eagle;

PatternExtractor3::PatternExtractor3(const cv::String& config) :
    _pattern_extractor(config) {

}

PatternExtractor3::PatternExtractor3(const Pattern& pattern) :
    _pattern_extractor(pattern) {

}

cloud3_t PatternExtractor3::extract(cv::Mat& img) {
    int t;
    return extract(img, t);
}

cloud3_t PatternExtractor3::extract(cv::Mat& img, int& id) {
    return extract(img, cv::Point2f(0, 0), id);
}

cloud3_t PatternExtractor3::extract(cv::Mat& img, const cv::Point2f& offset) {
    int t;
    return extract(img, offset, t);
}

std::vector<cloud3_t> PatternExtractor3::extract(std::vector<cv::Mat>& list) {
    std::vector<int> t;
    return extract(list, t);
}

std::vector<cloud3_t> PatternExtractor3::extract(std::vector<cv::Mat>& list, std::vector<int>& id) {
    std::vector<cloud3_t> patterns;
    patterns.reserve(list.size());
    id.clear(); id.reserve(list.size());
    int t;

    for (uint k = 0; k < list.size(); k++) {
        cloud3_t temp = extract(list[k], t);
        if (!(skip_invalid() && temp.empty())) {
            patterns.push_back(temp);
            id.push_back(t);
        } else {
            std::cout << "Pattern not found in image " << k << std::endl;
        }
    }

    return patterns;
}

std::vector<cloud3_t> PatternExtractor3::extract(std::vector<cv::Mat>& list, const std::vector<cv::Point2f>& offset) {
    std::vector<int> t;
    return extract(list, offset, t);
}

std::vector<cloud3_t> PatternExtractor3::extract(std::vector<cv::Mat>& list, const std::vector<cv::Point2f>& offset, std::vector<int>& id, bool display) {
    std::vector<cloud3_t> patterns;
    patterns.reserve(list.size());
    id.clear(); id.reserve(list.size());
    int t;

    for (uint k = 0; k < list.size(); k++) {
        cloud3_t temp = extract(list[k], offset[k], t, display);
        if (!(skip_invalid() && temp.empty())) {
            patterns.push_back(temp);
            id.push_back(t);
        } else {
            std::cout << "Pattern not found in image " << k << std::endl;
        }
    }

    return patterns;
}

std::vector<cloud3_t> PatternExtractor3::extractpath(const cv::String& path) {
    std::vector<int> t;
    return extractpath(path, t);
}

std::vector<cloud3_t> PatternExtractor3::extractpath(const cv::String& path, std::vector<int> id) {
    std::vector<cv::String> list;
    std::vector<cv::Mat> images;
    cv::Mat img;

    cv::glob(path + "/*.png", list);
    for (uint k = 0; k < list.size(); k++) {
        img = cv::imread(list[k]);
        if (!img.empty()) {
            images.push_back(img);
        }
    }

    return extract(images, id);
}
