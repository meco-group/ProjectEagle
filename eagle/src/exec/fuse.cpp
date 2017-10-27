#include <iostream>
#include <fstream>
#include <camera.h>
#include <projection.h>
#include <utils.h>

using namespace eagle;

void remapinf(std::string config, const cv::Mat& img, cv::Mat& warped, uint r, const cv::Size& siz, double z0 = 0) {
    // Init projection object
    Projection projection(config);

    // Rectify image
    cv::Mat remapped;
    projection.remap(img, remapped);

    // Construct homography matrix
    cv::Mat S = (cv::Mat_<float>(3,3) << r, 0, siz.width*0.5, 0, r, siz.height*0.5, 0, 0, 1);
    cv::Mat H = S*projection.get_homography(z0);
    
    // Warp image
    cv::warpPerspective(remapped, warped, H, siz);
}

//void overlay(const cv::Mat& in1, const cv::Mat& in2, const cv::Mat& out) {
//    cv::Mat mask1, white1, mask2, white2, overlap_mask, overlap;
//    
//    white1 = cv::Mat::ones()
//    cv::compare(mask1
//    cv::bitwise_and(in1, in2, out);
//
//}

int main(int argc, char* argv[]) {
    std::string config_path1 = (argc > 1) ? argv[1] : CONFIG_PATH;
    std::string config_path2 = (argc > 2) ? argv[2] : CONFIG_PATH;
    std::string images_path1 = (argc > 3) ? argv[3] : CAL_IMAGES_PATH;
    std::string images_path2 = (argc > 4) ? argv[4] : CAL_IMAGES_PATH;
    
    cv::Mat img;

    // read image 1 and remap
    img = cv::imread(images_path1);
    cv::Mat img1;
    remapinf(config_path1, img, img1, 200, cv::Size(1000, 1000), 0.0);

    // read image 2 and remap
    img = cv::imread(images_path2);
    cv::Mat img2;
    remapinf(config_path2, img, img2, 200, cv::Size(1000, 1000), 0.0);

    // blend images
    cv::addWeighted(img1, 0.5, img2, 0.5, 0, img);
    cv::imshow("blended",img);
    cv::waitKey(0);
    cv::imwrite("test_overlay.png", img);
}
