#include <iostream>
#include <fstream>
#include <camera.h>

#include <pnp_pattern_extractor3.h>
#include <planar_pattern_extractor3.h>
#include <utils.h>
#include <config_helper.h>

using namespace eagle;

/*
 * Compute the transform between the calibrated camera pair Camera1-Camera2
 * The transform is so that camera1 is the base and camera2 is expressed in the frame of camera1
 * p1 = R*p2+t
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
    std::vector<cloud3_t> points2 = extractor2->extractpath(images_path2);

    std::ofstream outputFile1("points1.txt");
    std::ofstream outputFile2("points2.txt");

    // use svd to compute the optimal transform
    cv::Mat P1(0,3,CV_32FC1);
    cv::Mat P2(0,3,CV_32FC1);
    for (uint k=0; k<points1.size(); k++) {
        if ((!points1[k].empty()) && (!points2[k].empty())) {
            cv::vconcat(P1,cv::Mat(points1[k].size(),3,CV_32FC1,points1[k].data()),P1);
            cv::vconcat(P2,cv::Mat(points2[k].size(),3,CV_32FC1,points2[k].data()),P2);
            for (uint j=0; j<points1[k].size(); j++) {
                outputFile1 << points1[k][j].x << '\t' << points1[k][j].y << '\t' << points1[k][j].z << std::endl;
                outputFile2 << points2[k][j].x << '\t' << points2[k][j].y << '\t' << points2[k][j].z << std::endl;
            }
        } else {
            std::cout << "image " << k << " valid" << std::endl;
        }
    }
    P1 = P1.t();
    P2 = P2.t();

    // compute centroids
    cv::Mat centroid1, centroid2;
    reduce(P1,centroid1,1,CV_REDUCE_AVG);
    reduce(P2,centroid2,1,CV_REDUCE_AVG);

    // shift point clouds w.r.t. centroids
    P1 = P1 - repeat(centroid1,1,P1.cols);
    P2 = P2 - repeat(centroid2,1,P2.cols);
    
    // compute svd of P1*P2'
    cv::Mat H = P2*(P1.t());
    cv::Mat U,S,Vt;
    cv::SVD::compute(H,S,U,Vt);
    
    // compute translation and rotation
    cv::Mat R, t;
    R = (U*Vt).t();
    if (cv::determinant(R) < 0) {
        cv::Mat c = R.col(2)*(-1.0f);
        c.copyTo(R.col(2));
    }
    t = -R*centroid2 + centroid1;

    //std::cout << "Rotation:" << R << std::endl << "Translation:" << t << std::endl;

    // compute reprojection error
    float total_error = 0.0;
    uint N = 0;

    for (uint k=0; k<points1.size(); k++) {
        if ((!points1[k].empty()) && (!points2[k].empty())) {
            for (uint j=0; j<points1[k].size(); j++) {
                cv::Mat p2t = R*cv::Mat(points2[k][j]) + t;
                total_error += cv::norm(points1[k][j] - cv::Point3f(p2t));
                N++;
            }
        }
    }

    total_error = total_error/N;
    std::cout << "<translation error>: " << total_error << "[m]" << std::endl;

    // Construct transformation matrix
    cv::Mat h1, h2, T21;
    cv::hconcat(R, t, h1);
    cv::hconcat(cv::Mat::zeros(1, 3, CV_32FC1), cv::Mat::ones(1, 1, CV_32FC1), h2);
    cv::vconcat(h1, h2, T21);
    T21.convertTo(T21, CV_64F); //

    // Load the extrinsic matrices of the already intrinsic device
    cv::Mat T10;
    cv::FileStorage fs = cv::FileStorage(config_path1, cv::FileStorage::READ);
    fs["camera"]["external_transformation"] >> T10;
    fs.release();
    

    cv::Mat T20 = T10*T21;
    std::cout << "Total external transformation matrix" << std::endl << T20 << std::endl;
    std::map<std::string, cv::Mat> mat_map({{"external_transformation", T20}});
    dump_matrices(config_path2, mat_map);
    set_integrated(config_path2, true);
}
