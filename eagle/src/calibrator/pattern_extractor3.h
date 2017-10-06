#ifndef PATTERN_EXTRACTOR3_H
#define PATTERN_EXTRACTOR3_H

#include <pattern_extractor.h>
#include <projection.h>

namespace eagle {

    typedef std::vector<cv::Point3f> cloud3_t;

    class PatternExtractor3
    {
        protected:
            PatternExtractor _pattern_extractor;
    
        public:
            PatternExtractor3(const cv::String& config);
            PatternExtractor3(const Pattern& pattern); 
    
            virtual cloud3_t extract(const cv::Mat& img, bool display = false) = 0;
            std::vector<cloud3_t> extract(const std::vector<cv::Mat>& list, bool display = false); 
            std::vector<cloud3_t> extractpath(const cv::String& path, bool display = false);

            void set_skip_invalid(bool skip = true) { _pattern_extractor.set_skip_invalid(skip); }
    };
};
    
#endif //PATTERN_EXTRACTOR3_H
