#include "pattern_extractor.h"

using namespace eagle;

PatternExtractor::PatternExtractor(const cv::String& config) :
    _pattern(config) {

}

PatternExtractor::PatternExtractor(const Pattern& pattern) :
    _pattern(pattern) {

}

cloud2_t PatternExtractor::extract(cv::Mat& img) {
    int t;
    cv::Point2f offset(0.0,0.0);
    return extract(img, offset, t, false);
}

cloud2_t PatternExtractor::extract(cv::Mat& img, int& id, bool display)
{
    cv::Point2f offset(0.0,0.0);
    return extract(img, offset, id, display);
}

cloud2_t PatternExtractor::extract(cv::Mat& img, const cv::Point2f& offset, bool display)
{
    int t;
    return extract(img, offset, t, display);
}

cloud2_t PatternExtractor::extract(cv::Mat& img, const cv::Point2f& offset, int& id, bool display) {
    cloud2_t points = _pattern.find(img, id, display);
    for (uint k = 0; k < points.size(); k++) {
        points[k] += offset;
    }

    if (display) {
        cv::imshow("Display",img);
        cv::waitKey(500);
    }

    return points;
}

std::vector<cloud2_t> PatternExtractor::extract(std::vector<cv::Mat>& list) {
    std::vector<int> t;
    return extract(list, t);
}

std::vector<cloud2_t> PatternExtractor::extract(std::vector<cv::Mat>& list, std::vector<int>& id, bool display)
{
    std::vector<cloud2_t> patterns;
    patterns.reserve(list.size());
    id.clear(); id.reserve(list.size());
    int tid;

    for (uint k=0; k<list.size(); k++) {
        cloud2_t temp = extract(list[k], tid, display);
        if (!(_skip_invalid && temp.empty())) {
            patterns.push_back(temp);
            id.push_back(tid);
        } else {
            std::cout << "Pattern not found in image " << k << std::endl;
        }
    }

    return patterns;

}

std::vector<cloud2_t> PatternExtractor::extract(std::vector<cv::Mat>& list, const std::vector<cv::Point2f>& offset, bool display)
{
    std::vector<int> t;
    return extract(list, offset, t, display);
}

std::vector<cloud2_t> PatternExtractor::extract(std::vector<cv::Mat>& list, const std::vector<cv::Point2f>& offset, std::vector<int>& id, bool display) {
    std::vector<cloud2_t> patterns;
    patterns.reserve(list.size());
    id.clear(); id.reserve(list.size());
    int tid;

    for (uint k = 0; k < list.size(); k++) {
        cloud2_t temp = extract(list[k], offset[k], tid, display);
        if (!(_skip_invalid && temp.empty())) {
            patterns.push_back(temp);
            id.push_back(tid);
        } else {
            std::cout << "Pattern not found in image " << k << std::endl;
        }
    }

    return patterns;
}

std::vector<cloud2_t> PatternExtractor::extract(const cv::String& path, bool display)
{
    std::vector<int> t;
    return extract(path, t, display);
}

std::vector<cloud2_t> PatternExtractor::extract(const cv::String& path, std::vector<int>& id, bool display) {
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

    return extract(images, id, display);
}
