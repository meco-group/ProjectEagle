#ifndef PROJECTION_H
#define PROJECTION_H

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv/cv.hpp>
#include <iostream>

namespace eagle {

    class Projection
    {
        private:
            cv::Mat _intrinsic_map1; //x
            cv::Mat _intrinsic_map2; //y
            //cv::Mat _extrinsic_map;
            cv::Mat _camera_matrix;
    
            void init(const cv::Mat& camera_matrix, const cv::Mat& distortion_vector, const cv::Size& size);
    
        public:
            Projection(const cv::Mat& camera_matrix, const cv::Mat& distortion_vector, const cv::Size& size);
            Projection(const cv::Mat& camera_matrix, const cv::Mat& distortion_vector, const cv::Mat& sample);
            Projection(const cv::String& config);
    
            // remap ground plane to image plane
            void remap(const cv::Mat& origin, cv::Mat& destination);
    
            // Projections to a world reference plane
            cv::Point3f project_to_plane(const cv::Point3f& i, const cv::Mat& plane);
            cv::Point3f project_to_plane(const cv::Point2f& i, const cv::Mat& plane);
            std::vector<cv::Point3f> project_to_plane(const std::vector<cv::Point3f> i, const cv::Mat& plane);
            std::vector<cv::Point3f> project_to_plane(const std::vector<cv::Point2f> i, const cv::Mat& plane);
            std::vector<std::vector<cv::Point3f>> project_to_plane(const std::vector<std::vector<cv::Point3f>> i, const cv::Mat& plane);
            std::vector<std::vector<cv::Point3f>> project_to_plane(const std::vector<std::vector<cv::Point2f>> i, const cv::Mat& plane);
    
            // Projection to the image plane
            cv::Point3f project_to_image3(const cv::Point3f& w);
            cv::Point2f project_to_image(const cv::Point3f& w);
            std::vector<cv::Point3f> project_to_image3(const std::vector<cv::Point3f>& w);
            std::vector<cv::Point2f> project_to_image(const std::vector<cv::Point3f>& w);

            // helpers
            cv::Mat camera_matrix() { return _camera_matrix; }
            cv::Size image_size() { return _intrinsic_map1.size(); }
    };
};

#endif //PROJECTION_H
