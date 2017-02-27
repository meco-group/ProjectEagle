#ifndef ROBOT_H
#define ROBOT_H


#include <iostream>
#include <stdint.h>
#include <math.h>
#include <string>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

class Robot {
    private:
        std::vector<double> _pose;
        std::vector<double> _ref_x;
        std::vector<double> _ref_y;
        std::string _name;
        std::vector<int> _global_marker_locations_px;
        cv::RotatedRect _box;
        bool _detected;
        int _width;
        int _height;
        std::vector<double> _local_marker_locations_m;

        void createBox();
        std::vector<double> transform(const std::vector<double>& point, int pixelspermeter, int height);
        std::vector<double> invtransform(const std::vector<double>& point, int pixelspermeter, int height);
        void mixWithWhite(const cv::Scalar& color, cv::Scalar& color_w, double perc_white);

    public:
        Robot(int width, int height, std::vector<double>& local_marker_locations_m);
        void reset();
        void setMarkers(const std::vector<int>& marker_locations_px);
        std::string getName();
        bool detected();
        cv::RotatedRect getBox();
        double getBottomTopDistance();
        void getMarkers(std::vector<double>& marker_vector, int pixelspermeter, int height);
        void setPose(const std::vector<double>& pose);
        void setRef(const std::vector<double>& ref_x, const std::vector<double>& ref_y);
        void getTopMarkers(std::vector<cv::Mat>& top_markers);
        void draw(cv::Mat& frame, const cv::Scalar& color, int pixelspermeter);
};

#endif //ROBOT_H
