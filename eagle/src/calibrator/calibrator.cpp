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
    _square_size = fs["calibrator"]["square_size"];
    int board_width = fs["calibrator"]["board_width"];
    int board_height = fs["calibrator"]["board_height"];
    _board_size = cv::Size(board_width, board_height);
    _calib_fix_principal_point = ((int)fs["calibrator"]["calib_fix_principal_point"] == 1);
    _calib_zero_tangent_dist = ((int)fs["calibrator"]["calib_zero_tangent_dist"] == 1);
    _calib_fix_aspect_ratio = ((int)fs["calibrator"]["calib_fix_aspect_ratio"] == 1);
    const char *pattern_strings[] = {"INVALID", "PICAM", "OPICAM", "SEE3CAM", "LATCAM", "OCAM"};
    _calibration_pattern = get_pattern(fs["calibrator"]["pattern"]);
}

bool Calibrator::execute(const std::string& images_path) {
    cv::glob(images_path + "/*.png",  _img_list);
    _img_pnts.clear();
    _object_pnts.clear();
    cv::Mat img;
    for (uint i=0; i<_img_list.size(); i++) {
        // get next image
        img = cv::imread(_img_list[i]);
        if (img.empty()) {
            continue;
        }
        // process image
        std::vector<cv::Point2f> pnt_buffer;
        if (process_image(img, pnt_buffer)) {
            _img_pnts.push_back(pnt_buffer);
        } else {
            std::cout << "Ditching image " << _img_list[i] << "." << std::endl;
            continue;
        }
        // get object points
        std::vector<cv::Point3f> object_buffer;
        process_pattern(object_buffer);
        _object_pnts.push_back(object_buffer);
    }
    // calibrate camera
    _img_size = img.size();
    std::vector<cv::Mat> rvecs, tvecs;
    if (_img_pnts.size() && calibrate(_object_pnts, _img_pnts, _img_size, _camera_matrix, _distortion_vector, _ground_plane, rvecs, tvecs)) {
        double reproj_err = compute_reprojection_error(_object_pnts, _img_pnts,
            _camera_matrix, _distortion_vector, rvecs, tvecs);
        double scaling_err = compute_scaling_error(_img_pnts, _camera_matrix,
            _distortion_vector, _ground_plane);
        std::cout << "Camera calibration succeeded." << std::endl;
        std::cout << "\t* average reprojection error: " << reproj_err << " px" << std::endl;
        std::cout << "\t* relative scaling error: " << scaling_err << std::endl;
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

std::vector<std::vector<cv::Point3f>> Calibrator::object_points() {
    return _object_pnts;
}

std::vector<std::vector<cv::Point2f>> Calibrator::image_points() {
    return _img_pnts;
}

cv::Size Calibrator::image_size() {
    return _img_size;
}

bool Calibrator::process_image(cv::Mat& img, std::vector<cv::Point2f>& pnt_buffer) {
    bool found;
    switch (_calibration_pattern) {
        case CHESSBOARD:
            found = findChessboardCorners(img, _board_size, pnt_buffer, 0);
            if (found) {
                // improve the found coners' coordinate accuracy
                cv::Mat img_gray;
                cv::cvtColor(img, img_gray, cv::COLOR_BGR2GRAY);
                cv::cornerSubPix(img_gray, pnt_buffer, _board_size, cv::Size(-1,-1),
                    cv::TermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1));
                // draw the corners
                cv::drawChessboardCorners(img, _board_size, cv::Mat(pnt_buffer), found);
            }
            break;
        case CIRCLES_GRID:
            found = findCirclesGrid(img, _board_size, pnt_buffer);
            break;
        case ASYMMETRIC_CIRCLES_GRID:
            found = findCirclesGrid(img, _board_size, pnt_buffer, cv::CALIB_CB_ASYMMETRIC_GRID);
            break;
        default:
            found = false;
            break;
    }
    imshow("Image", img);
    cv::waitKey(500);
    return found;
};

void Calibrator::process_pattern(std::vector<cv::Point3f> &object_buffer) {
    // Reset the corners
    object_buffer.clear();

    // Calculate grid points based on pattern
    switch(_calibration_pattern) {
        case CHESSBOARD:
        case CIRCLES_GRID:
            for (int i=0; i<_board_size.height; i++) {
                for (int j=0; j<_board_size.width; j++) {
                    object_buffer.push_back(cv::Point3f(float(j*_square_size), float(i*_square_size), 0));
                }
            }
            break;
        case ASYMMETRIC_CIRCLES_GRID:
            for (int i=0; i <_board_size.height; i++) {
                for (int j = 0; j <_board_size.width; j++) {
                    object_buffer.push_back(cv::Point3f(float((2*j+i%2)*_square_size), float(i*_square_size), 0));
                }
            }
            break;
        default:
            break;
    }
}

bool Calibrator::calibrate(const std::vector<std::vector<cv::Point3f>>& object_pnts,
    const std::vector<std::vector<cv::Point2f>>& img_pnts,  const cv::Size& img_size,
    cv::Mat& camera_matrix, cv::Mat& distortion_vector, cv::Mat& ground_plane,
    std::vector<cv::Mat>& rvecs, std::vector<cv::Mat>& tvecs) {
    // init flag
    int flag = 0;
    if (_calib_fix_principal_point) flag |= CV_CALIB_FIX_PRINCIPAL_POINT;
    if (_calib_zero_tangent_dist) flag |= CV_CALIB_ZERO_TANGENT_DIST;
    if (_calib_fix_aspect_ratio) flag |= CV_CALIB_FIX_ASPECT_RATIO;
    // initialize camera_matrix & distortion_vector
    camera_matrix = cv::Mat::eye(3, 3, CV_64F);
    if (flag & CV_CALIB_FIX_ASPECT_RATIO) {
        camera_matrix.at<double>(0, 0) = 1.0;
    }
    distortion_vector = cv::Mat::zeros(8, 1, CV_64F);
    // find intrinsic and extrinsic camera parameters
    cv::calibrateCamera(object_pnts, img_pnts, img_size, camera_matrix,
        distortion_vector, rvecs, tvecs, flag|CV_CALIB_FIX_K4|CV_CALIB_FIX_K5);
    // get ground plane
    get_ground_plane(rvecs, tvecs, ground_plane);
    // check the result
    return (cv::checkRange(camera_matrix) && cv::checkRange(distortion_vector));
}

double Calibrator::compute_reprojection_error(const std::vector<std::vector<cv::Point3f>>& object_pnts,
    const std::vector<std::vector<cv::Point2f>>& img_pnts, const cv::Mat& camera_matrix,
    const cv::Mat& distortion_vector, const std::vector<cv::Mat>& rvecs, const std::vector<cv::Mat>& tvecs) {
    std::vector<cv::Point2f> img_pnts2;
    double total_error = 0;
    double total_pnts = 0;
    uint n = object_pnts.size();
    std::vector<double> errors(n);
    for (uint i=0; i<n; i++) {
        cv::projectPoints(cv::Mat(object_pnts[i]), rvecs[i], tvecs[i],
            _camera_matrix, _distortion_vector, img_pnts2);
        double error = cv::norm(cv::Mat(img_pnts[i]), cv::Mat(img_pnts2), CV_L2);
        errors[i] = sqrt(error*error/n);
        total_error += error*error;
        total_pnts += n;
    }
    return sqrt(total_error/total_pnts);
}

double Calibrator::compute_scaling_error(const std::vector<std::vector<cv::Point2f>>& img_pnts,
    const cv::Mat& camera_matrix, const cv::Mat& distortion_vector, const cv::Mat& ground_plane) {
    double total_sum = 0.0;
    int total_cnt = 0;
    cv::Mat undistorted;
    for (uint i=0; i<img_pnts.size(); i++) {
        cv::undistortPoints(cv::Mat(img_pnts[i]), undistorted,
            cv::Mat::eye(3, 3, CV_64F), distortion_vector);
        double sum = 0.0;
        int cnt = 0;
        for (int j=1; j<img_pnts[i].size(); j++) {
            cv::Point3f i1(img_pnts[i][j].x, img_pnts[i][j].y, 1);
            cv::Point3f P1;
            cv::Point3f i0(img_pnts[i][j-1].x, img_pnts[i][j-1].y, 1);
            cv::Point3f P0;
            project_to_ground(i1, P1, camera_matrix, ground_plane);
            project_to_ground(i0, P0, camera_matrix, ground_plane);
            double dist = cv::norm(cv::Mat(P1), cv::Mat(P0));
            if (cnt == 0 || (dist <= 1.2*(sum/cnt))) {
                sum += dist;
                cnt++;
            }
        }
        // std::cout << "Average square distance image " << i << ": " << (sum/cnt) << std::endl;
        total_sum += sum;
        total_cnt += cnt;
    }
    double square_size = total_sum/total_cnt;
    // std::cout << "Global average square size: " << square_size << std::endl;
    // std::cout << "Relative error: " << (_square_size-square_size)/_square_size << std::endl;
    return (_square_size-square_size)/_square_size;
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

void Calibrator::project_to_image(cv::Point3f& i, const cv::Point3f& w, cv::Mat& camera_matrix) {
    cv::Mat p = camera_matrix*cv::Mat(w);
    p = p*(1.0/p.at<double>(2, 0));
    i = cv::Point3f(p.at<double>(0, 0), p.at<double>(1, 0), p.at<double>(2, 0));
}

Calibrator::Pattern Calibrator::get_pattern(const std::string& pattern) {
    Pattern result = INVALID;
    if (!pattern.compare("CHESSBOARD")) result = CHESSBOARD;
    if (!pattern.compare("CIRCLES_GRID")) result = CIRCLES_GRID;
    if (!pattern.compare("ASYMMETRIC_CIRCLES_GRID")) result = ASYMMETRIC_CIRCLES_GRID;
    return result;
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
