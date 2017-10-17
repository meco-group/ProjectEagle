#ifndef PATTERN_EXTRACTOR_H
#define PATTERN_EXTRACTOR_H

#include <opencv/cv.hpp>
#include <pattern.h>
#include <iostream>

namespace eagle {

    typedef std::vector<cv::Point2f> cloud2_t;

    class PatternExtractor
    {
        private:
            Pattern _pattern;
            bool _skip_invalid = true;
    
        public:
            PatternExtractor(const cv::String& config);
            PatternExtractor(const Pattern& pattern);
    
            cloud2_t extract(cv::Mat& img);
            cloud2_t extract(cv::Mat& img, int& id);
            cloud2_t extract(cv::Mat& img, const cv::Point2f& offset);
            cloud2_t extract(cv::Mat& img, const cv::Point2f& offset, int& id, bool display = false);

            std::vector<cloud2_t> extract(std::vector<cv::Mat>& list);
            std::vector<cloud2_t> extract(std::vector<cv::Mat>& list, std::vector<int>& id);
            std::vector<cloud2_t> extract(std::vector<cv::Mat>& list, const std::vector<cv::Point2f>& offset);
            std::vector<cloud2_t> extract(std::vector<cv::Mat>& list, const std::vector<cv::Point2f>& offset, std::vector<int>& id, bool display = false);

            std::vector<cloud2_t> extract(const cv::String& path);
            std::vector<cloud2_t> extract(const cv::String& path, std::vector<int>& id);
    
            Pattern pattern() { return _pattern; }
            void set_skip_invalid(bool skip = true) { _skip_invalid = skip; }
            bool skip_invalid() { return _skip_invalid; }
    };
};
    
#endif //PATTERN_EXTRACTOR_H
