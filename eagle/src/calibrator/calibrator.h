#ifndef CALIBRATOR_H
#define CALIBRATOR_H

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.hpp>
#include <iostream>
#include <map>

namespace eagle {

    class Calibrator {

        private:
            enum Pattern { INVALID, CHESSBOARD, CIRCLES_GRID, ASYMMETRIC_CIRCLES_GRID };

            bool _executed;
            Pattern _calibration_pattern;
            double _square_size;
            cv::Size _board_size;
            bool _calib_fix_principal_point;
            bool _calib_zero_tangent_dist;
            bool _calib_fix_aspect_ratio;
            std::vector<cv::String> _img_list;

            void read_parameters(const cv::FileStorage& fs);
            bool process_image(cv::Mat& img, std::vector<cv::Point2f>& pnt_buffer);
            void process_pattern(std::vector<cv::Point3f> &object_buffer);
            bool calibrate(const std::vector<std::vector<cv::Point3f>>& object_pnts,
                const std::vector<std::vector<cv::Point2f>>& img_pnts, const cv::Size& img_size,
                cv::Mat& camera_matrix, cv::Mat& distortion_vector, cv::Mat& ground_plane,
                std::vector<cv::Mat>& rvecs, std::vector<cv::Mat>& tvecs);
            double compute_reprojection_error(const std::vector<std::vector<cv::Point3f>>& object_pnts,
                const std::vector<std::vector<cv::Point2f>>& img_pnts, const cv::Mat& camera_matrix,
                const cv::Mat& distortion_vector, const std::vector<cv::Mat>& rvecs, const std::vector<cv::Mat>& tvecs);
            double compute_scaling_error(const std::vector<std::vector<cv::Point2f>>& img_pnts,
                const cv::Mat& camera_matrix, const cv::Mat& distortion_vector, const cv::Mat& ground_plane);
            void get_ground_plane(const std::vector<cv::Mat>& rvecs, const std::vector<cv::Mat>& tvecs, cv::Mat& ground_plane);
            void project_to_ground(const cv::Point3f& i, cv::Point3f& w, cv::Mat camera_matrix, const cv::Mat& ground_plane);
            void project_to_image(cv::Point3f& i, const cv::Point3f& w, cv::Mat& camera_matrix);
            Pattern get_pattern(const std::string& pattern);

            cv::Mat _camera_matrix;
            cv::Mat _distortion_vector;
            cv::Mat _ground_plane;
            std::vector<std::vector<cv::Point3f>> _object_pnts;
            std::vector<std::vector<cv::Point2f>> _img_pnts;
            cv::Size _img_size;

        public:
            Calibrator(const std::string& config_path);
            bool execute(const std::string& images_path);
            bool save(const std::string& config_path);
            std::vector<std::vector<cv::Point3f>> object_points();
            std::vector<std::vector<cv::Point2f>> image_points();
            cv::Size image_size();
            static void dump_matrices(const std::string& xml_path, std::map<std::string, cv::Mat>& matrices);
            static void set_calibrated(const std::string& xml_path, bool value);
            static void set_integrated(const std::string& xml_path, bool value);
    };

};

#endif // CALIBRATOR_H
