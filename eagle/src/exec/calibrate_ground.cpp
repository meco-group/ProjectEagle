#include <iostream>
#include <fstream>
#include <map>
#include <opencv/cv.hpp>

#include <pnp_pattern_extractor3.h>
#include <planar_pattern_extractor3.h>
#include <transform.h>
#include <utils.h>
#include <config_helper.h>

using namespace eagle;

/*
 * Estimate the ground plane based on a RANSAC algorithm. There should be at least 
 * 70% inliers for the algorithm to be effective.
 */

std::vector<double> model_cost(const std::vector<cloud3_t>& points, const cv::Mat& plane);
void compute_reprojection_ground(const cv::String& config_path, const cv::String& image_path, bool display);

int main(int argc, char* argv[]) {
    // For now only translation is taken into account. Rotated images are not yet.
    // We still have to write a routine to compute the rotation between the image plane and the world plane.
    /*double z = 0.;
    std::cout << tvecs.size() << std::endl;
    for (int k=0; k<tvecs.size(); k++) {
    std::cout << "here3" << std::endl;
        z += tvecs[k].at<double>(2, 0);
    std::cout << "here4" << std::endl;
    }
    z /= tvecs.size();
    ground_plane = cv::Mat::zeros(1, 4, CV_64F);
    ground_plane.at<double>(0, 2) = 1.;
    ground_plane.at<double>(0, 3) = -z;*/

    std::string config_path = (argc > 1) ? argv[1] : CONFIG_PATH;
    std::string images_path = (argc > 2) ? argv[2] : CAL_IMAGES_PATH;

    // extract image points
    PnpPatternExtractor3 pnp_extractor(config_path);
    pnp_extractor.set_skip_invalid(true);
    pnp_extractor.set_transform(cv::Mat::eye(4,4,CV_32F));
    std::vector<cloud3_t> points = pnp_extractor.extractpath(images_path, false);

    // Ransac algorithm for robust outlier detection
    cv::Mat plane = cv::Mat::zeros(3, 1, CV_64F);
    double optimal_cost = 1.0; //cost will always be samller than 0.01..
    cv::Mat optimal_plane;
    uint64_t seed = std::chrono::system_clock::now().time_since_epoch()/std::chrono::milliseconds(1);
    cv::RNG rng(seed);
    cv::Mat A = cv::Mat(3,3,CV_64F);
    cv::Mat b = -cv::Mat::ones(3,1,CV_64F);

    for (uint k=0; k<40; k++) {
        std::cout << "iter: " << k << " | ";
        // Randomly pick a plane
        for (uint j=0; j<3; j++) {
            int cloud = rng.uniform(0,points.size());
            int element = rng.uniform(0,points[cloud].size());
            cv::Mat(cv::Mat(points[cloud][element]).t()).copyTo(A.row(j));
        }
        cv::solve(A,b,plane);
        
        // compute number of inliers
        int noi = 0;
        cv::Mat Ain(0,3,CV_32F);
        std::vector<double> distances = model_cost(points, plane);
        for (uint j=0; j<distances.size(); j++) {
            if (distances[j] < 0.05) { //consider it to be an inlier
                noi++;
                cv::vconcat(Ain,cv::Mat(points[k].size(),3,CV_32F,points[k].data()),Ain);
            }
        }

        // Least-squares approximate model
        std::cout << "initial noi: " << noi << " | ";
        if (noi > points.size()/3) {
            Ain.convertTo(Ain,CV_64F);
            cv::Mat bls = -cv::Mat::ones(Ain.rows,1,CV_64F);
            cv::solve((Ain.t())*(Ain),(Ain.t())*bls,plane);

            noi = 0;
            double cost = 0.0;
            distances = model_cost(points, plane);
            for (uint j=0; j<distances.size(); j++) {
                if (distances[j] < 0.05) { //consider it to be an inlier
                    noi++;
                    cost += distances[j];
                }
            }
            cost /= noi;
            std::cout << "eventual noi: " << noi << " | cost: " << cost << " | best cost: " << optimal_cost;

            if ((cost < optimal_cost) && (noi > points.size()/2)) {
                optimal_cost = cost;
                plane.copyTo(optimal_plane);
            }
        }
        std::cout << std::endl;
    }

    // saving ground plane
    cv::hconcat(optimal_plane.t(),cv::Mat::ones(1,1,CV_64F),optimal_plane);

    // compute the extrinsic transform to the ground
    double cx = optimal_plane.at<double>(0,0);
    double cy = optimal_plane.at<double>(0,1);
    double cz = optimal_plane.at<double>(0,2);
    double d = optimal_plane.at<double>(0,3);
    double gamma = -d/cz;
    double rx = -(cx+d)/cz;
    double t = 1.0/std::sqrt(1.0+(rx-gamma)*(rx-gamma));
    cv::Point3f O1(0,0,0);
    cv::Point3f O2(0,0,gamma);
    cv::Point3f ex1(1,0,0);
    cv::Point3f ex2(1,0,rx); ex2 = ex2*t;
    cv::Point3f ez1(0,0,1);
    cv::Point3f ez2(cx,cy,cz); ez2 = ez2/norm(ez2); 
    if (cz >= 0)
        ez2 = -ez2;
    ez2 += O2;

    std::vector<cv::Point3f> points1 {O1, ex1, ez1};
    std::vector<cv::Point3f> points2 {O2, ex2, ez2};
    cv::Mat T = compute_transform(points1, points2);
    T.convertTo(T,CV_64F);

    // Save everything to the config file
    std::cout << "ground plane " << optimal_plane << std::endl;
    std::cout << "Extrinsic transform: "<< T << std::endl;
    std::map<std::string, cv::Mat> matrices;
    matrices["ground_plane"] = optimal_plane;
    matrices["external_transformation"] = T;
    dump_matrices(config_path, matrices);
    std::cout << "Saved ground plane information succesfully." << std::endl;
    
    // compute reprojection to ground
    //compute_reprojection_ground(config_path, images_path, false);
}

std::vector<double> model_cost(const std::vector<cloud3_t>& points, const cv::Mat& plane) {
    std::vector<double> distances;
    for (uint j=0; j<points.size(); j++) {
        double distance = 0.0;
        for (uint i=0; i<points[j].size(); i++) {
            cv::Mat p = cv::Mat(points[j][i]); 
            p.convertTo(p,CV_64F);
            cv::Mat num = plane.t()*p;
            cv::Mat den = plane.t()*plane;
            double temp = std::abs(num.at<double>(0,0)+1.0)/std::sqrt(den.at<double>(0,0));
            //std::cout << p << plane << temp << std::endl;
            distance += temp;
        }
        distances.push_back(distance/points[j].size());
    }

    return distances;
}

void compute_reprojection_ground(const cv::String& config_path, const cv::String& image_path, bool display) {
    double translation_error = 0.0;
    double scaling_error = 0.0;
    uint translation_N = 0;
    uint scaling_N = 0;

    // construct Planar pattern extractor
    PlanarPatternExtractor3 planar_extractor(config_path);
    planar_extractor.set_skip_invalid(false);
    std::vector<cloud3_t> planar_points = planar_extractor.extractpath(image_path, display);

    // construct pnp pattern extractor
    PnpPatternExtractor3 pnp_extractor(config_path);
    pnp_extractor.set_skip_invalid(false);
    std::vector<cloud3_t> pnp_points = pnp_extractor.extractpath(image_path, display);

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
    CalibrationPattern* p = (CalibrationPattern*)(planar_extractor.pattern().pattern());
    double dim = p->dimension();
    scaling_error = ((scaling_error/scaling_N)-dim)/dim;
    std::cout << "<Translation error> = " << translation_error << std::endl;
    std::cout << "<Scaling error> = " << scaling_error << std::endl;
}
