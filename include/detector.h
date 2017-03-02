#ifndef DETECTOR_H
#define DETECTOR_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include "robot.h"
#include "obstacle.h"

class Detector {
    private:
        cv::FileStorage _params;
        cv::SimpleBlobDetector _blob_detector;
        cv::Mat _background;
        cv::Mat _cont_mask;
        cv::Size _frame_size;

        void init_blob();
        void init_background();
        bool subtract_background(const cv::Mat& frame, std::vector<std::vector<cv::Point>>& contours);
        void detect_robots(const cv::Mat& frame, const std::vector<std::vector<cv::Point>>& contours, const std::vector<Robot*>& robots);
        void find_robots(cv::Mat& roi, const cv::Point2f& roi_location, const std::vector<Robot*>& robots);
        void decode_robot(const cv::Mat& roi, const cv::Point2f& roi_location, const std::vector<cv::Point2f>& points, const std::vector<Robot*>& robots);
        bool get_markers(const std::vector<cv::Point2f>& points, std::vector<cv::Point2f>& markers);
        bool subtract_robots(std::vector<std::vector<cv::Point>>& contours, const std::vector<Robot*>& robots);
        void detect_obstacles(const cv::Mat& frame, const std::vector<std::vector<cv::Point> >& contours, const std::vector<Robot*>& robots, std::vector<Obstacle*>& obstacles);
        void filter_obstacles(const std::vector<std::vector<cv::Point>>& contours, const std::vector<Robot*>& robots, std::vector<Obstacle*>& obstacles);
        void sort_obstacles(std::vector<Obstacle*>& obstacles);

        std::vector<cv::Point2f> cam2worldframe(const std::vector<cv::Point2f>& points);
        cv::Point2f cam2worldframe(const cv::Point2f& point);
        std::vector<cv::Point2f> world2camframe(const std::vector<cv::Point2f>& points);
        cv::Point2f world2camframe(const cv::Point2f& point);

    public:
        Detector(const std::string& param_file);
        void search(const cv::Mat& frame, const std::vector<Robot*>& robots, std::vector<Obstacle*>& obstacles);
        void draw(cv::Mat& frame, const std::vector<Robot*>& robots, const std::vector<Obstacle*>& obstacles);
};

#endif //DETECTOR_H
