#include <pattern_extractor3.h>

using namespace eagle;

PatternExtractor3::PatternExtractor3(const cv::String& config) :
    _pattern_extractor(config)
{

}

PatternExtractor3::PatternExtractor3(const Pattern& pattern) :
    _pattern_extractor(pattern)
{

}

std::vector<cloud3_t> PatternExtractor3::extract(const std::vector<cv::Mat>& list, bool display) {
    std::vector<cloud3_t> patterns;
    patterns.reserve(list.size());

    for (uint k=0; k<list.size(); k++) {
        cloud3_t temp = extract(list[k], display);
        if (!(skip_invalid() && temp.empty())) {
            patterns.push_back(temp);
        } else {
            std::cout << "Pattern not found in image " << k << std::endl;
        }
    }
    
    return patterns;
}

std::vector<cloud3_t> PatternExtractor3::extractpath(const cv::String& path, bool display) {
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
