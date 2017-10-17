#ifndef PROJECTION_H
#define PROJECTION_H

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv/cv.hpp>
#include <iostream>
#include <transform.h>

namespace eagle {

    class Projection
    {
        private:
            cv::Mat _intrinsic_map1; //x
            cv::Mat _intrinsic_map2; //y
            //cv::Mat _extrinsic_map;
            cv::Mat _camera_matrix;
            cv::Mat _T = cv::Mat::eye(4,4,CV_32F);
    
            void init(const cv::Mat& camera_matrix, const cv::Mat& distortion_vector, const cv::Size& size, const cv::Mat& T = cv::Mat::eye(4,4,CV_32F));
    
        public:
            Projection(const cv::Mat& camera_matrix, const cv::Mat& distortion_vector, const cv::Size& size, const cv::Mat& T = cv::Mat::eye(4,4,CV_32F));
            Projection(const cv::Mat& camera_matrix, const cv::Mat& distortion_vector, const cv::Mat& sample, const cv::Mat& T = cv::Mat::eye(4,4,CV_32F));
            Projection(const cv::String& config);
    
            // remap ground plane to image plane
            void remap(const cv::Mat& origin, cv::Mat& destination);
    
            // Projections to a world reference plane
            cv::Point3f project_to_plane(const cv::Point3f& i, const cv::Mat& plane);
            cv::Point3f project_to_plane(const cv::Point2f& i, const cv::Mat& plane);
            cv::Point3f project_to_plane(const cv::Point& i, const cv::Mat& plane);
            std::vector<cv::Point3f> project_to_plane(const std::vector<cv::Point3f> i, const cv::Mat& plane);
            std::vector<cv::Point3f> project_to_plane(const std::vector<cv::Point2f> i, const cv::Mat& plane);
            std::vector<cv::Point3f> project_to_plane(const std::vector<cv::Point> i, const cv::Mat& plane);
            std::vector<std::vector<cv::Point3f>> project_to_plane(const std::vector<std::vector<cv::Point3f>> i, const cv::Mat& plane);
            std::vector<std::vector<cv::Point3f>> project_to_plane(const std::vector<std::vector<cv::Point2f>> i, const cv::Mat& plane);
            std::vector<std::vector<cv::Point3f>> project_to_plane(const std::vector<std::vector<cv::Point>> i, const cv::Mat& plane);
    
            // Projection to the image plane
            cv::Point3f project_to_image3(const cv::Point3f& w);
            cv::Point2f project_to_image(const cv::Point3f& w);
            std::vector<cv::Point3f> project_to_image3(const std::vector<cv::Point3f>& w);
            std::vector<cv::Point2f> project_to_image(const std::vector<cv::Point3f>& w);

            // helpers
            cv::Mat camera_matrix() { return _camera_matrix; }
            cv::Size image_size() { return _intrinsic_map1.size(); }
            cv::Mat get_transform() const { return _T; }
            void set_transform(const cv::Mat& T);
    };
};

#endif //PROJECTION_H
