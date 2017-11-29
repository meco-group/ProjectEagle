#include <iostream>
#include <fstream>
#include <eagle.h>

using namespace eagle;

int main(int argc, char* argv[]) {
    std::string config_path1 = (argc > 1) ? argv[1] : CONFIG_PATH;
    std::string config_path2 = (argc > 2) ? argv[2] : CONFIG_PATH;
    std::string images_path1 = (argc > 3) ? argv[3] : CAL_IMAGES_PATH;
    std::string images_path2 = (argc > 4) ? argv[4] : CAL_IMAGES_PATH;

    cv::Mat img;
    int pixels_per_meter = 150; //px/m
    cv::Size img_size = cv::Size(6, 6); //m
    double height = 0.0;

    // read image 1 and remap
    img = cv::imread(images_path1);
    cv::Mat img1;
    cv::Point2f offset1;
    Projection projection1(config_path1);
    projection1.remap(img,img);
    cv::Point2f o = projection1.project_to_image(cv::Point3f(0,0,height));
    cv::circle(img, o, 10, cv::Scalar(127,127,127), -1); //mark the origin
    remapinf_cropped(config_path1, img, img1, pixels_per_meter, offset1, height);
    //remapinf(config_path1, img, img1, pixels_per_meter, img_size, height);

    // read image 2 and remap
    img = cv::imread(images_path2);
    cv::Mat img2;
    cv::Point2f offset2;
    Projection projection2(config_path2);
    projection2.remap(img,img);
    o = projection2.project_to_image(cv::Point3f(0,0,height));
    cv::circle(img, o, 10, cv::Scalar(127,127,127), -1); //mark the origin
    remapinf_cropped(config_path2, img, img2, pixels_per_meter, offset2, height);
    //remapinf(config_path2, img, img2, pixels_per_meter, img_size, height);

    // blend images
    img = cv::Mat(img_size*pixels_per_meter, CV_8UC3);
    overlay(img1, offset1, img2, offset2, img);
    //overlay(img1, img2, img);

    // replace by update
    /*img1.copyTo(img);
    replace(img2, img);*/

    cv::imshow("blended", img);
    cv::waitKey(0);
    cv::imwrite("test_overlay.png", img);
}
