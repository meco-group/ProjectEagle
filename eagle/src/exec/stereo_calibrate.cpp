#include "calibrator.h"
#include "camera.h"

using namespace eagle;

void save_external_tf(const std::string& config_path, const cv::Mat& T) {
    std::ifstream file_in(config_path);
    std::string data;
    std::vector<std::string> lines;
    std::vector<double> mat_vec;
    bool found;
    while (getline(file_in, data)) {
        found = false;

        if (data.find("external_transformation") != std::string::npos) {
            lines.push_back(data + "\n");
            for (int k=0; k<3; k++) {
                getline(file_in, data);
                lines.push_back(data + "\n");
            }
            getline(file_in, data);
            data = "        data: [";
            mat_vec.assign((double*)(T).datastart,
                (double*)(T).dataend);
            for (int k=0; k<mat_vec.size(); k++) {
                data += std::to_string(mat_vec[k]);
                if (k < mat_vec.size()-1) {
                    data += ", ";
                }
            }
            data += "]\n";
            lines.push_back(data);
        } else {
            lines.push_back(data + "\n");
        }
    }
    file_in.close();
    std::ofstream file_out(config_path);
    for (int k=0; k<lines.size(); k++) {
        file_out << lines[k];
    }
    file_out.close();
}

int main(int argc, char* argv[]) {
    std::string config_path1 = ""; // fill in
    std::string config_path2 = "";

    // calibrate the first camera
    Calibrator cal1(config_path1);
    cal1.execute();
    // cal1.save(config_path1);

    // calibrate the second camera
    Calibrator cal2(config_path1);
    cal2.execute();
    // cal2.save(config_path2);

    // TODO we could get cameraMatrix, distCoeffs from cal file (from intrinsic calibration)
    cv::FileStorage fs1(config_path1, cv::FileStorage::READ);
    cv::FileStorage fs2(config_path2, cv::FileStorage::READ);
    cv::Mat K1, K2, D1, D2;
    fs1["camera"]["camera_matrix"] >> K1;
    fs1["camera"]["distortion_vector"] >> D1;
    fs2["camera"]["camera_matrix"] >> K2;
    fs2["camera"]["distortion_vector"] >> D2;
    fs2.release();
    fs1.release();

    // Stereo calibration
    // t2 = R12 * t1 + t12
    // R2 = R12 * R1
    cv::Mat R12, t12, T12, F, E, t2;
    cv::stereoCalibrate(cal1.object_points(), cal1.image_points(), cal2.image_points(), K1, D1, K2, D2, cal1.image_size(), R12, t12, F, E);
    std::cout << R12 << std::endl << t12 << std::endl;
    cv::hconcat(R12, t12, T12);
    cv::hconcat(cv::Mat::zeros(1, 3, CV_64F), cv::Mat::ones(1, 1, CV_64F), t2);
    cv::vconcat(T12, t2, T12);

    // Load the extrinsic matrices of the already intrinsic device
    cv::Mat T10;
    fs1 = cv::FileStorage(config_path1, cv::FileStorage::READ);
    fs1["external_transformation"] >> T10;
    fs1.release();

    // We need to calculate R20 and T20
    // So that we can calculate T0 and R0 when we measure T2 and R2
    // t0 = R20 * t2 + t20
    // R0 = R20 * R2
    // i.e. T0 = T20*T2
    // ruben doesn't get this??
    cv::Mat T20 = T10*T12.inv();
    std::cout << T20 << std::endl;
    save_external_tf(config_path2, T20);
}
