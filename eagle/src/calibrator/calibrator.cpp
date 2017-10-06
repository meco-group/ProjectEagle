#include "calibrator.h"
#include "rapidxml.hpp"
#include "rapidxml_print.hpp"
#include <fstream>
#include <map>

using namespace eagle;

Calibrator::Calibrator(const std::string& config_path) : _pattern(config_path), _executed(false) {
    cv::FileStorage fs(config_path, cv::FileStorage::READ);
    read_parameters(fs);
    fs.release();
};

void Calibrator::read_parameters(const cv::FileStorage& fs) {
    _calib_fix_principal_point = ((int)fs["calibrator"]["calib_fix_principal_point"] == 1);
    _calib_zero_tangent_dist = ((int)fs["calibrator"]["calib_zero_tangent_dist"] == 1);
    _calib_fix_aspect_ratio = ((int)fs["calibrator"]["calib_fix_aspect_ratio"] == 1);

    int width = fs["camera"]["resolution"]["width"];
    int height = fs["camera"]["resolution"]["height"];
    _img_size = cv::Size(width,height);
}

bool Calibrator::execute(const std::string& images_path, bool display) {
    cv::Mat img;
    preprocess(images_path, display);
    // calibrate camera
    std::vector<cv::Mat> rvecs, tvecs;
    if (_image_pnts.size() && calibrate(rvecs, tvecs)) {
        compute_reprojection_ground(images_path, display);
        
        _executed = true;
        return true;
    } else {
        std::cout << "Camera calibration failed." << std::endl;
        return false;
    }
}
bool Calibrator::save(const std::string& config_path) {
    if (!_executed) {
        return false;
    }
    std::map<std::string, cv::Mat> matrices;
    matrices["camera_matrix"] = _camera_matrix;
    matrices["distortion_vector"] = _distortion_vector;
    matrices["ground_plane"] = _ground_plane;
    Calibrator::dump_matrices(config_path, matrices);
    Calibrator::set_calibrated(config_path, true);
    std::cout << "Saved calibration information succesfully." << std::endl;
    return true;
}

void Calibrator::preprocess(const std::string& images_path, bool display)
{
    // create pattern extractor
    PatternExtractor extractor(_pattern);
    // extract pattern
    _image_pnts = extractor.extract(images_path, false);
    _pattern_pnts = extractor.pattern().reference(_image_pnts.size());
}

bool Calibrator::calibrate(std::vector<cv::Mat>& rvecs, std::vector<cv::Mat>& tvecs) {
    // init flag
    int flag = 0;
    if (_calib_fix_principal_point) flag |= CV_CALIB_FIX_PRINCIPAL_POINT;
    if (_calib_zero_tangent_dist) flag |= CV_CALIB_ZERO_TANGENT_DIST;
    if (_calib_fix_aspect_ratio) flag |= CV_CALIB_FIX_ASPECT_RATIO;
    // initialize camera_matrix & distortion_vector
    cv::Mat camera_matrix = cv::Mat::eye(3, 3, CV_64F);
    if (flag & CV_CALIB_FIX_ASPECT_RATIO) {
        camera_matrix.at<double>(0, 0) = 1.0;
    }
    cv::Mat distortion_vector = cv::Mat::zeros(8, 1, CV_64F);
    cv::Mat ground_plane = cv::Mat::zeros(4, 1, CV_64F);

    // find intrinsic and extrinsic camera parameters
    double reproj_error = cv::calibrateCamera(_pattern_pnts, _image_pnts, image_size(), camera_matrix,
        distortion_vector, rvecs, tvecs, flag|CV_CALIB_FIX_K4|CV_CALIB_FIX_K5);
    std::cout << "Calibration rms error: " << reproj_error << std::endl;
    
    // get ground plane
    get_ground_plane(rvecs, tvecs, ground_plane);
    
    // check the result
    if(cv::checkRange(camera_matrix) && cv::checkRange(distortion_vector)) {
        camera_matrix.copyTo(_camera_matrix);
        distortion_vector.copyTo(_distortion_vector);
        ground_plane.copyTo(_ground_plane);

        // print the parameters
        std::cout << "camera matrix " << _camera_matrix << std::endl;
        std::cout << "distortion vector " << _distortion_vector << std::endl;
        std::cout << "ground plane " << _ground_plane << std::endl;

        return true;
    } else {
        return false;
    }
}

void Calibrator::compute_reprojection_ground(const cv::String& path, bool display) {
    double translation_error = 0.0;
    double scaling_error = 0.0;
    uint translation_N = 0;
    uint scaling_N = 0;

    // construct Planar pattern extractor
    Projection projection(_camera_matrix, _distortion_vector, image_size());
    PlanarPatternExtractor3 planar_extractor = PlanarPatternExtractor3(_pattern, projection, _ground_plane);
    planar_extractor.set_skip_invalid(false);
    std::vector<cloud3_t> planar_points = planar_extractor.extractpath(path, display);

    // construct pnp pattern extractor
    PnpPatternExtractor3 pnp_extractor = PnpPatternExtractor3(_pattern, _camera_matrix, _distortion_vector);
    pnp_extractor.set_skip_invalid(false);
    std::vector<cloud3_t> pnp_points = pnp_extractor.extractpath(path, display);

    // Compute errors
    for (uint i=0; i<planar_points.size(); i++) {
        if (!planar_points[i].empty()) { //valid point cloud
            if (!pnp_points[i].empty()) { // another valid point cloud
                // compute translation error for origin point
                translation_error += cv::norm(planar_points[i][0] - pnp_points[i][0]);
                translation_N++;
            } else {
                std::cout << "Skipping pnp_points " << i << std::endl;
            }

            // compute scaling error
            for(uint j=1; j<planar_points[i].size(); j++){
                float distance = cv::norm(planar_points[i][j]-planar_points[i][j-1]);
                if (scaling_N == 0 || (distance <= 1.2*(scaling_error/scaling_N))) {
                    scaling_error += distance;
                    scaling_N++;
                }
            }
        } else {
            std::cout << "Skipping planar_points " << i << std::endl;
        }
    }

    translation_error = translation_error/translation_N;
    scaling_error = ((scaling_error/scaling_N)-_pattern.dimension())/_pattern.dimension();
    std::cout << "<Translation error> = " << translation_error << std::endl;
    std::cout << "<Scaling error> = " << scaling_error << std::endl;
}

void Calibrator::get_ground_plane(const std::vector<cv::Mat>& rvecs, const std::vector<cv::Mat>& tvecs, cv::Mat& ground_plane) {
    // For now only translation is taken into account. Rotated images are not yet.
    // We still have to write a routine to compute the rotation between the image plane and the world plane.
    double z = 0.;
    for (int k=0; k<tvecs.size(); k++) {
        z += tvecs[k].at<double>(2, 0);
    }
    z /= tvecs.size();
    ground_plane = cv::Mat::zeros(1, 4, CV_64F);
    ground_plane.at<double>(0, 2) = 1.;
    ground_plane.at<double>(0, 3) = -z;
}

void Calibrator::dump_matrices(const std::string& xml_path, std::map<std::string, cv::Mat>& matrices) {
    using namespace rapidxml;
    std::ifstream ifile(xml_path);
    std::vector<char> buffer((std::istreambuf_iterator<char>(ifile)), std::istreambuf_iterator<char>());
    buffer.push_back('\0');
    ifile.close();
    xml_document<> doc;
    doc.parse<0>(&buffer[0]);
    xml_node<>* camera_node = doc.first_node("opencv_storage")->first_node("camera");
    std::map<std::string, cv::Mat>::iterator it;
    xml_node<>* node;
    std::string data;
    std::vector<double> mat_vec;
    for (it=matrices.begin(); it != matrices.end(); it++) {
        mat_vec.assign((double*)(it->second).datastart, (double*)(it->second).dataend);
        data = "";
        for (int k=0; k<mat_vec.size(); k++) {
            data += std::to_string(mat_vec[k]);
            data += " ";
        }
        node = camera_node->first_node((it->first).c_str());
        char* node_name = doc.allocate_string("data");
        char* node_value = doc.allocate_string(data.c_str());
        node->remove_node((node->first_node("data")));
        node->append_node(doc.allocate_node(node_element, node_name, node_value));
    }
    std::ofstream ofile(xml_path);
    ofile << "<?xml version=\"1.0\"?>\n";
    ofile << doc;
    ofile.close();
}

void Calibrator::set_calibrated(const std::string& xml_path, bool value) {
    using namespace rapidxml;
    std::ifstream ifile(xml_path);
    std::vector<char> buffer((std::istreambuf_iterator<char>(ifile)), std::istreambuf_iterator<char>());
    buffer.push_back('\0');
    ifile.close();
    xml_document<> doc;
    doc.parse<0>(&buffer[0]);
    xml_node<>* calibrator_node = doc.first_node("opencv_storage")->first_node("calibrator");
    calibrator_node->remove_node((calibrator_node->first_node("calibrated")));
    calibrator_node->append_node(doc.allocate_node(node_element, "calibrated", (value) ? "1" : "0"));
    std::ofstream ofile(xml_path);
    ofile << "<?xml version=\"1.0\"?>\n";
    ofile << doc;
    ofile.close();
}

void Calibrator::set_integrated(const std::string& xml_path, bool value) {
    using namespace rapidxml;
    std::ifstream ifile(xml_path);
    std::vector<char> buffer((std::istreambuf_iterator<char>(ifile)), std::istreambuf_iterator<char>());
    buffer.push_back('\0');
    ifile.close();
    xml_document<> doc;
    doc.parse<0>(&buffer[0]);
    xml_node<>* calibrator_node = doc.first_node("opencv_storage")->first_node("calibrator");
    calibrator_node->remove_node((calibrator_node->first_node("integrated")));
    calibrator_node->append_node(doc.allocate_node(node_element, "integrated", (value) ? "1" : "0"));
    std::ofstream ofile(xml_path);
    ofile << "<?xml version=\"1.0\"?>\n";
    ofile << doc;
    ofile.close();
}
