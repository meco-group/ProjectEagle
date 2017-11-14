#include <iostream>
#include <fstream>
#include <eagle.h>

using namespace eagle;

/*
 * Compute the transform between the calibrated camera pair Camera1-Camera2
 * The transform is so that camera1 is the base and camera2 is expressed in the frame of camera1
 * p1 = R*p2+t
 * taken from: http://nghiaho.com/?page_id=671
 */

int main(int argc, char* argv[]) {
    std::string config_path1 = (argc > 1) ? argv[1] : CONFIG_PATH;
    std::string config_path2 = (argc > 2) ? argv[2] : CONFIG_PATH;
    std::string images_path1 = (argc > 3) ? argv[3] : CAL_IMAGES_PATH;
    std::string images_path2 = (argc > 4) ? argv[4] : CAL_IMAGES_PATH;

    // read the data images and extract 3d point cloud
    PatternExtractor3 *extractor1 = new PnpPatternExtractor3(config_path1);
    extractor1->set_skip_invalid(false);
    std::vector<cloud3_t> points1 = extractor1->extractpath(images_path1);
    PatternExtractor3 *extractor2 = new PnpPatternExtractor3(config_path2);
    extractor2->set_skip_invalid(false);
    extractor2->set_transform(cv::Mat::eye(4, 4, CV_32F));
    std::vector<cloud3_t> points2 = extractor2->extractpath(images_path2);

    // Compute the optimal transform
    cv::Mat Tc2_to_w = compute_transform(points1, points2);
    std::cout << "Local transform: " << Tc2_to_w << std::endl;

    // compute reprojection error
    float total_error = 0.0;
    uint N = 0;

    for (uint k = 0; k < points1.size(); k++) {
        if ((!points1[k].empty()) && (!points2[k].empty())) {
            for (uint j = 0; j < points1[k].size(); j++) {
                total_error += cv::norm(points1[k][j] - transform(Tc2_to_w, points2[k][j]));
                N++;
            }
        }
    }

    total_error = total_error / N;
    std::cout << "<translation error>: " << total_error << "[m]" << std::endl;

    // Load the extrinsic matrices of the already intrinsic device
    Tc2_to_w.convertTo(Tc2_to_w, CV_64F); //
    cv::Mat Tc1_to_w, ground;
    cv::FileStorage fs = cv::FileStorage(config_path1, cv::FileStorage::READ);
    fs["camera"]["external_transformation"] >> Tc1_to_w;
    fs["camera"]["ground_plane"] >> ground;
    fs.release();

    cv::Mat Tc2_to_c1 = Tc1_to_w.inv() * Tc2_to_w;
    std::cout << "Cam2Cam transformation matrix: " << Tc2_to_c1 << std::endl;
    std::cout << "Total external transformation matrix: " << std::endl << Tc2_to_w << std::endl;

    // Add the ground plane
    std::cout << "Equation of the ground plane: " << std::endl << ground << std::endl;

    std::map<std::string, cv::Mat> mat_map({{"external_transformation", Tc2_to_w}, {"ground_plane", ground}});
    dump_matrices(config_path2, mat_map);
    set_integrated(config_path2, true);
}
