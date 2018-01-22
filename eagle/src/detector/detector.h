#ifndef DETECTOR_H
#define DETECTOR_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include "robot.h"
#include "obstacle_circle.h"
#include "obstacle_rectangle.h"
#include "planar_pattern_extractor3.h"
#include "pnp_pattern_extractor3.h"
#include "transform.h"

namespace eagle {

class Detector {

  private:
    cv::Mat _background;
    int _th_bg_subtraction;
    cv::Mat _cont_mask;

    Projection _projection;
    cv::Mat _marker_plane;

    Pattern _pat;
    PnpPatternExtractor3* _extr;
    int _verbose;

    cv::Mat get_mask(const cv::Mat& frame);
    std::vector<std::vector<cv::Point>> get_contours(const cv::Mat& frame);
    void detect_robots(const cv::Mat& frame, const std::vector<std::vector<cv::Point>>& contours, const std::vector<Robot*>& robots);
    void subtract_robots(cv::Mat& mask, const std::vector<Robot*>& robots);
    void detect_obstacles(const cv::Mat& frame, const std::vector<std::vector<cv::Point> >& contours, const std::vector<Robot*>& robots, std::vector<Obstacle*>& obstacles);
    void filter_obstacles(const std::vector<std::vector<cv::Point>>& contours, const std::vector<Robot*>& robots, std::vector<Obstacle*>& obstacles);
    void sort_obstacles(std::vector<Obstacle*>& obstacles);
    void convert_robots(const std::vector<Robot*>& robots, std::vector<Obstacle*>& obstacles);

  public:
    Detector(const std::string& config_path, const cv::Mat& background = cv::Mat(0, 0, CV_8UC3));
    void search(const cv::Mat& frame, const std::vector<Robot*>& robots, std::vector<Obstacle*>& obstacles, bool obstacle_detection=true);
    cv::Mat draw(cv::Mat& frame, const std::vector<Robot*>& robots, const std::vector<Obstacle*>& obstacles);
    void set_background(const cv::Mat& bg);
    void verbose(int verbose);

    Projection *projection() { return &_projection; }

};

};

#endif //DETECTOR_H
