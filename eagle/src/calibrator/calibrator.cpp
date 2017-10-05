#include "calibrator.h"
#include "rapidxml.hpp"
#include "rapidxml_print.hpp"
#include <fstream>
#include <map>

using namespace eagle;

Calibrator::Calibrator(const std::string& config_path) : _executed(false) {
    cv::FileStorage fs(config_path, cv::FileStorage::READ);
    read_parameters(fs);
    fs.release();
};

void Calibrator::read_parameters(const cv::FileStorage& fs) {
    _calib_fix_principal_point = ((int)fs["calibrator"]["calib_fix_principal_point"] == 1);
    _calib_zero_tangent_dist = ((int)fs["calibrator"]["calib_zero_tangent_dist"] == 1);
    _calib_fix_aspect_ratio = ((int)fs["calibrator"]["calib_fix_aspect_ratio"] == 1);

    // Initialize pattern
    int rows = fs["calibrator"]["board_height"];
    int cols = fs["calibrator"]["board_width"];
    double dimension = fs["calibrator"]["square_size"];
    set_pattern(fs["calibrator"]["pattern"],rows,cols,dimension);
}

bool Calibrator::execute(const std::string& images_path, bool display) {
    cv::Mat img;
    preprocess(images_path, display);
    // calibrate camera
    std::vector<cv::Mat> rvecs, tvecs;
    if (_image_pnts.size() && calibrate(rvecs, tvecs)) {
        double reproj_err = compute_reprojection_error(rvecs, tvecs);
        double scaling_err = compute_scaling_error();
        double ground_reproj_err = compute_reprojection_ground(rvecs, tvecs);
        std::cout << "Camera calibration succeeded." << std::endl;
        std::cout << "\t* average reprojection error: " << reproj_err << " px" << std::endl;
        std::cout << "\t* average ground reprojection error: " << ground_reproj_err << " m" << std::endl;
        std::cout << "\t* relative scaling error: " << scaling_err << std::endl;
        
        _executed = true;
        return true;
    } else {
        std::cout << "Camera calibration failed." << std::endl;
        return false;
    }
}

double Calibrator::compute_reprojection_ground(const std::vector<cv::Mat>& rvecs, const std::vector<cv::Mat>& tvecs, bool display) {
    cv::Mat img, undist, new_camera_matrix;
    double total_error = 0.0;
    uint N = 0;

    for (uint i=0; i<_img_list.size(); i++) { //_img_list.size()
        img = cv::imread(_img_list[i]);
        undistort_full(img, undist, new_camera_matrix);
        std::vector<cv::Point2f> pnt_buffer;
        if(display){
            cv::imshow("dist",img);
            cv::imshow("undist",undist);
            cv::waitKey(1000);
        }
        if(_pattern.find_pattern(undist, pnt_buffer, false)){
            cv::Point3f P;
            project_to_ground(pnt_buffer[0], P, new_camera_matrix, _ground_plane);
            total_error += cv::norm(P - cv::Point3f(tvecs[i]));
            N++;
        } else {
            std::cout << "Pattern not found in " << _img_list[i] << std::endl;
        }
    }

    return (total_error/N);
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
    cv::glob(images_path + "/*.png",  _img_list);
    _image_pnts.clear();
    _pattern_pnts.clear();
    cv::Mat img;

    uint i=0;
    while ( i<_img_list.size() ) {
        // get next image
        img = cv::imread(_img_list[i]);
        if (img.empty()) {
            continue;
        }
        // process image
        std::vector<cv::Point2f> image_pnts;
        if (_pattern.find_pattern(img, image_pnts, display)) {
            _image_pnts.push_back(image_pnts);
            i++;
        } else {
            std::cout << "Ditching image " << _img_list[i] << "." << std::endl;
            _img_list.erase(_img_list.begin()+i);
            continue;
        }
        // get object points
        std::vector<cv::Point3f> pattern_pnts;
        _pattern.reference_pattern(pattern_pnts);
        _pattern_pnts.push_back(pattern_pnts);
    }
    _img_size = img.size();
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
    std::cout << "Calibration rms error: " << cv::calibrateCamera(_pattern_pnts, _image_pnts, image_size(), camera_matrix,
        distortion_vector, rvecs, tvecs, flag|CV_CALIB_FIX_K4|CV_CALIB_FIX_K5) << std::endl;
    // get ground plane
    get_ground_plane(rvecs, tvecs, ground_plane);
    // check the result
    if(cv::checkRange(camera_matrix) && cv::checkRange(distortion_vector)) {
        camera_matrix.copyTo(_camera_matrix);
        distortion_vector.copyTo(_distortion_vector);
        ground_plane.copyTo(_ground_plane);
        return true;
    } else {
        return false;
    }
}

double Calibrator::compute_reprojection_error(const std::vector<cv::Mat>& rvecs, const std::vector<cv::Mat>& tvecs) {
    std::vector<cv::Point2f> image_pnts_reproj;
    double total_error = 0;
    double total_pnts = 0;
    int n = _pattern_pnts.size();
    for (uint i=0; i<n; i++) {
        cv::projectPoints(cv::Mat(_pattern_pnts[i]), rvecs[i], tvecs[i],
            _camera_matrix, _distortion_vector, image_pnts_reproj);

        double error = cv::norm(cv::Mat(_image_pnts[i]), cv::Mat(image_pnts_reproj), CV_L2);
        total_error += error*error;
        total_pnts += n;
    }
    return sqrt(total_error/total_pnts);
}

double Calibrator::compute_scaling_error() {
    cv::Mat img, undist;
    double sum = 0.0;
    double distance;
    uint N = 0;

    for (uint i=0; i<_img_list.size(); i++) {
        img = cv::imread(_img_list[i]);
        cv::Mat new_camera_matrix;
        undistort_full(img, undist, new_camera_matrix);
        std::vector<cv::Point2f> pnt_buffer;
        if(_pattern.find_pattern(undist, pnt_buffer, false)){
            std::cout << _img_list[i] << std::endl;
            cv::Point3f P0, P1;
            std::vector<cv::Point3f> points;
            project_to_ground(pnt_buffer[0], P0, new_camera_matrix, _ground_plane);
            points.push_back(P0);
            for(uint j=1; j<pnt_buffer.size(); j++){
                project_to_ground(pnt_buffer[j], P1, new_camera_matrix, _ground_plane);
                distance = cv::norm(P1-P0);
                if (N == 0 || (distance <= 1.2*(sum/N))) {
                    sum += distance;
                    N++;
                }
                P0 = P1;
                points.push_back(P0);
            }
            _world_pnts.push_back(points);
        }
    }

    return (sum/N - _pattern.dimension())/_pattern.dimension();
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

void Calibrator::project_to_ground(const cv::Point3f& i, cv::Point3f& w, cv::Mat camera_matrix, const cv::Mat& ground_plane) {
    // Project image coordinates to a plane ground
    // i is of the form [x;y;1], w is of the form [X,Y,Z], K is the camera matrix
    // ground holds the coefficients of the ground plane [a,b,c,d] where the
    // ground plane is represented by ax+by+cz+d=0

    cv::Mat M1, M2, A, i_m;
    cv::Mat(i).convertTo(i_m, CV_64F);
    cv::hconcat(-cv::Mat::eye(3,3,CV_64F), camera_matrix.inv()*i_m, M1);
    cv::hconcat(ground_plane(cv::Rect(0,0,3,1)), cv::Mat::zeros(1, 1, CV_64F), M2);
    cv::vconcat(M1, M2, A);
    cv::Mat b = cv::Mat::zeros(4,1,CV_64F);
    b.at<double>(3, 0) = -ground_plane.at<double>(0, 3);
    cv::Mat W = A.inv()*b;
    w = cv::Point3f(W(cv::Rect(0, 0, 1, 3)));
}
void Calibrator::project_to_ground(const cv::Point2f& i, cv::Point3f& w, cv::Mat camera_matrix, const cv::Mat& ground_plane) {
    cv::Point3f i3 = cv::Point3f(i.x, i.y, 1.0f);
    project_to_ground(i3, w, camera_matrix, ground_plane);
}

void Calibrator::project_to_image(cv::Point3f& i, const cv::Point3f& w, cv::Mat& camera_matrix) {
    cv::Mat p = camera_matrix*cv::Mat(w);
    p = p*(1.0/p.at<double>(2, 0));
    i = cv::Point3f(p.at<double>(0, 0), p.at<double>(1, 0), p.at<double>(2, 0));
}

void Calibrator::set_pattern(const std::string& type, const int rows, const int cols, const double dimension) {
    if (!type.compare("CHESSBOARD")) _pattern = Pattern::Chessboard(rows, cols, dimension);
    if (!type.compare("CIRCLES_GRID")) _pattern = Pattern::Circles(rows, cols, dimension);
    if (!type.compare("ASYMMETRIC_CIRCLES_GRID")) _pattern = Pattern::AsymCircles(rows, cols, dimension);
}

void Calibrator::undistort_full(const cv::Mat& original, cv::Mat& destination, cv::Mat& new_camera_matrix)
{
    cv::Mat m = cv::getOptimalNewCameraMatrix(_camera_matrix, _distortion_vector, image_size(), 1);
    m.copyTo(new_camera_matrix);

    cv::Mat map1, map2;
    cv::initUndistortRectifyMap(_camera_matrix, _distortion_vector, cv::Mat::eye(3,3,CV_32F), new_camera_matrix, image_size(), CV_16SC2, map1, map2);

    cv::remap(original, destination, map1, map2, cv::INTER_LINEAR);
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
