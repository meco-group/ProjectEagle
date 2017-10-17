#ifndef CIRCLE_TRIANGLE_H
#define CIRCLE_TRIANGLE_H

#include <pattern_interface.h>
#include <opencv2/opencv.hpp>

namespace eagle {

    class CircleTriangle : public PatternInterface
    {
        private:
            cv::Point2f _dimension;
            cv::Point2f _qr_size;
            cv::Point2f _qr_pos;
            cv::Point2i _qr_n;

            bool check(std::vector<cv::Point2f>& points) const;
            int decode(cv::Mat& img, const std::vector<cv::Point2f>& points) const;
            float ratio() const { return _dimension.x/_dimension.y; }

            // search algorithm settings
            float _th_top_marker = 0.1;
            float _th_triangle_ratio = 0.1;

        public:
            CircleTriangle(const cv::String& config);
            CircleTriangle(const cv::Point2f& dimension, const cv::Point2f& qr_size, const cv::Point2f& qr_pos = cv::Point2f(0,0), const cv::Point2i& qr_n = cv::Point2i(2,2));

            virtual std::vector<cv::Point2f> find(cv::Mat& img) const;
            virtual std::vector<cv::Point2f> find(cv::Mat& img, int& id, bool draw) const;
            virtual std::vector<cv::Point3f> reference() const;
    
    };
};

#endif
