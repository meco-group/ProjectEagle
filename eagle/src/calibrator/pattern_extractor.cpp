#include "pattern_extractor.h"

using namespace eagle;

PatternExtractor::PatternExtractor(const cv::String& config) :
    _pattern(config)
{

}

PatternExtractor::PatternExtractor(const Pattern& pattern) :
    _pattern(pattern)
{

}

cloud2_t PatternExtractor::extract(const cv::Mat& img, bool display) {
    return _pattern.find(img, display);
}

std::vector<cloud2_t> PatternExtractor::extract(const std::vector<cv::Mat>& list, bool display) {
    std::vector<cloud2_t> patterns;
    patterns.reserve(list.size());

    for (uint k=0; k<list.size(); k++) {
        cloud2_t temp = extract(list[k], display);
        if (!(_skip_invalid && temp.empty())) {
            patterns.push_back(temp);
        } else {
            std::cout << "Pattern not found in image " << k << std::endl;
        }
    }

    return patterns;
}

std::vector<cloud2_t> PatternExtractor::extract(const cv::String& path, bool display) {
    std::vector<cv::String> list;
    std::vector<cv::Mat> images;
    cv::Mat img;

    cv::glob(path + "/*.png", list);
    for (uint k=0; k<list.size(); k++) {
        img = cv::imread(list[k]);
        if (!img.empty()) {
            images.push_back(img);
        }
    }

    return extract(images, display);
}
