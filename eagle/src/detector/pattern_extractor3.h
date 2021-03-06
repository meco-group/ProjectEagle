#ifndef PATTERN_EXTRACTOR3_H
#define PATTERN_EXTRACTOR3_H

#include "pattern_extractor.h"
#include "projection.h"

namespace eagle {

typedef std::vector<cv::Point3f> cloud3_t;

class PatternExtractor3 {
  protected:
    PatternExtractor _pattern_extractor;

  public:
    PatternExtractor3(const cv::String& config);
    PatternExtractor3(const Pattern& pattern);

    virtual cloud3_t extract(cv::Mat& img);
    virtual cloud3_t extract(cv::Mat& img, int& id);
    virtual cloud3_t extract(cv::Mat& img, const cv::Point2f& offset);
    virtual cloud3_t extract(cv::Mat& img, const cv::Point2f& offset, int& id, bool display = false) = 0;

    virtual std::vector<cloud3_t> extract(std::vector<cv::Mat>& list);
    virtual std::vector<cloud3_t> extract(std::vector<cv::Mat>& list, std::vector<int>& id);
    virtual std::vector<cloud3_t> extract(std::vector<cv::Mat>& list, const std::vector<cv::Point2f>& offset);
    virtual std::vector<cloud3_t> extract(std::vector<cv::Mat>& list, const std::vector<cv::Point2f>& offset, std::vector<int>& id, bool display = false);


    std::vector<cloud3_t> extractpath(const cv::String& path);
    std::vector<cloud3_t> extractpath(const cv::String& path, std::vector<int> id);

    void set_skip_invalid(bool skip = true) { _pattern_extractor.set_skip_invalid(skip); }
    bool skip_invalid() { return _pattern_extractor.skip_invalid(); }
    Pattern pattern() { return _pattern_extractor.pattern(); }
    virtual void set_transform(const cv::Mat& T) = 0;
};
};

#endif //PATTERN_EXTRACTOR3_H
