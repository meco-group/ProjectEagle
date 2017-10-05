#ifndef CALIBRATOR_H
#define CALIBRATOR_H

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.hpp>
#include <iostream>
#include <map>
#include <pattern.h>

namespace eagle {

    class Calibrator {

        private:
            bool _executed;
            Pattern _pattern;
            bool _calib_fix_principal_point;
            bool _calib_zero_tangent_dist;
            bool _calib_fix_aspect_ratio;
            std::vector<cv::String> _img_list;

            void read_parameters(const cv::FileStorage& fs);
            void preprocess(const std::string& images_path, bool display = true);
            void postprocess(bool display = false);
            bool calibrate(std::vector<cv::Mat>& rvecs, std::vector<cv::Mat>& tvecs);
            double compute_reprojection_error(const std::vector<cv::Mat>& rvecs, const std::vector<cv::Mat>& tvecs);
            double compute_scaling_error();
            double compute_reprojection_ground(const std::vector<cv::Mat>& rvecs, const std::vector<cv::Mat>& tvecs, bool display = false);
            void get_ground_plane(const std::vector<cv::Mat>& rvecs, const std::vector<cv::Mat>& tvecs, cv::Mat& ground_plane);
            void project_to_ground(const cv::Point3f& i, cv::Point3f& w, cv::Mat camera_matrix, const cv::Mat& ground_plane);
            void project_to_ground(const cv::Point2f& i, cv::Point3f& w, cv::Mat camera_matrix, const cv::Mat& ground_plane);
            void project_to_image(cv::Point3f& i, const cv::Point3f& w, cv::Mat& camera_matrix);
            void undistort_full(const cv::Mat& original, cv::Mat& destination, cv::Mat& new_camera_matrix);
            
            Pattern get_pattern() { return _pattern; };
            void set_pattern(const std::string& pattern, const int rows, const int cols, const double dimension);

            cv::Mat _camera_matrix;
            cv::Mat _undist_camera_matrix;
            cv::Mat _distortion_vector;
            cv::Mat _ground_plane;
            std::vector<std::vector<cv::Point3f>> _pattern_pnts;
            std::vector<std::vector<cv::Point2f>> _image_pnts;
            std::vector<std::vector<cv::Point3f>> _world_pnts;
            cv::Size _img_size;

        public:
            Calibrator(const std::string& config_path);
            bool execute(const std::string& images_path, bool display = true);
            bool save(const std::string& config_path);
            std::vector<std::vector<cv::Point3f>> object_points() { return _pattern_pnts; }
            std::vector<std::vector<cv::Point2f>> image_points() { return _image_pnts; }
            std::vector<std::vector<cv::Point3f>> world_points() { return _world_pnts; }
            cv::Size image_size() { return _img_size; }

            static void dump_matrices(const std::string& xml_path, std::map<std::string, cv::Mat>& matrices);
            static void set_calibrated(const std::string& xml_path, bool value);
            static void set_integrated(const std::string& xml_path, bool value);

            
    };

};

#endif // CALIBRATOR_H
