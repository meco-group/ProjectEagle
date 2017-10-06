#ifndef PLANAR_PATTERN_EXTRACTOR3_H
#define PLANAR_PATTERN_EXTRACTOR3_H

#include <pattern_extractor3.h>

namespace eagle {

    class PlanarPatternExtractor3 : public PatternExtractor3
    {
        private:
            Projection _projection;
            cv::Mat _plane;
    
        public:
            PlanarPatternExtractor3(const cv::String& config);
            PlanarPatternExtractor3(const Pattern& pattern, const Projection& projection, const cv::Mat& plane);
    
            virtual cloud3_t extract(const cv::Mat& img, bool display = false);
            
            Projection projection() { return _projection; }
            cv::Mat Plane() { return _plane; }
    
    
    };
};

#endif //PLANAR_PATTERN_EXTRACTOR3_H
