#ifndef DETECTOR_H
#define DETECTOR_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include "robot.h"
#include "obstacle.h"

namespace eagle {

    class Detector {

        private:
            cv::Ptr<cv::SimpleBlobDetector> _blob_detector;
            cv::Mat _background;
            cv::Mat _cont_mask;
            cv::Matx23f _cam2world_tf;
            cv::Matx23f _world2cam_tf;

            // parameters
            double _pixel2meter;
            double _meter2pixel;
            double _min_robot_area;
            double _min_obstacle_area;
            double _triangle_ratio;
            double _qr_posx;
            double _qr_posy;
            double _qr_sizex;
            double _qr_sizey;
            int _qr_nbitx;
            int _qr_nbity;
            double _th_triangle_ratio;
            double _th_top_marker;
            int _th_bg_subtraction;

            void read_parameters(const cv::FileStorage& fs);
            void init_blob(const cv::FileStorage& fs);
            bool subtract_background(const cv::Mat& frame, std::vector<std::vector<cv::Point>>& contours);
            void detect_robots(const cv::Mat& frame, const std::vector<std::vector<cv::Point>>& contours, const std::vector<Robot*>& robots);
            void find_robots(cv::Mat& roi, const cv::Point2f& roi_location, const std::vector<Robot*>& robots);
            void decode_robot(const cv::Mat& roi, const cv::Point2f& roi_location, const std::vector<cv::Point2f>& points, const std::vector<Robot*>& robots);
            bool get_markers(const std::vector<cv::Point2f>& points, std::vector<cv::Point2f>& markers);
            bool subtract_robots(std::vector<std::vector<cv::Point>>& contours, const std::vector<Robot*>& robots);
            void detect_obstacles(const cv::Mat& frame, const std::vector<std::vector<cv::Point> >& contours, const std::vector<Robot*>& robots, std::vector<Obstacle*>& obstacles);
            void filter_obstacles(const std::vector<std::vector<cv::Point>>& contours, const std::vector<Robot*>& robots, std::vector<Obstacle*>& obstacles);
            void sort_obstacles(std::vector<Obstacle*>& obstacles);
            void init_transformations(const cv::Matx33f& c2w);

            std::vector<cv::Point2f> cam2worldframe(const std::vector<cv::Point2f>& points);
            cv::Point2f cam2worldframe(const cv::Point2f& point);
            std::vector<cv::Point2f> world2camframe(const std::vector<cv::Point2f>& points);
            cv::Point2f world2camframe(const cv::Point2f& point);

        public:
            Detector(const std::string& config_path, const cv::Matx33f& cam2world_tf);
            void search(const cv::Mat& frame, const std::vector<Robot*>& robots, std::vector<Obstacle*>& obstacles);
            void draw(cv::Mat& frame, const std::vector<Robot*>& robots, const std::vector<Obstacle*>& obstacles);

    };

};

#endif //DETECTOR_H
