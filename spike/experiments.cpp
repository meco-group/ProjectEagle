#include "cam/libcam.h"
#include "examples_config.h"
#include "detector.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

int main(void) {
    // Setup the camera
    EXAMPLE_CAMERA_T cam(EXAMPLE_CAMERA_INDEX);
    cam.setBrightness(7);
    cam.calibrate("../config/see3cam.yml"); //camera can be calibrated
    cam.start();

    // Load background
    cv::Mat background = cv::imread("background.png", CV_LOAD_IMAGE_COLOR);
    if (!background.data) {
        std::cout << "Could not open " << "background.png" << "!" << std::endl;
    }

    // read some data from detector file
    cv::FileStorage params("../config/detector.yml", cv::FileStorage::READ);

    // Extract all settings
    // General
    double _meter2pixel = (double)params["pixelspermeter"];
    double _pixel2meter = 1./_meter2pixel;
    double _min_robot_area = (double)params["min_robot_area"];
    double _min_obstacle_area = (double)params["min_obstacle_area"];
    double _triangle_ratio = (double)params["markers"]["triangle_ratio"];
    double _qr_posx = (double)params["markers"]["qr_posx"];
    double _qr_posy = (double)params["markers"]["qr_posy"];
    double _qr_sizex = (double)params["markers"]["qr_sizex"];
    double _qr_sizey = (double)params["markers"]["qr_sizey"];
    int _qr_nbitx = (int)params["markers"]["qr_nbitx"];
    int _qr_nbity = (int)params["markers"]["qr_nbity"];
    double _th_triangle_ratio = (double)params["thresholds"]["triangle_ratio"];
    double _th_top_marker = (double)params["thresholds"]["top_marker"];
    // int _th_bg_subtraction = (int)params["thresholds"]["bg_subtraction"];
    int _th_bg_subtraction = 50;

    // Blob
    cv::SimpleBlobDetector::Params blob_par;
    blob_par.minThreshold = (int)params["blob"]["minThreshold"];
    blob_par.maxThreshold = (int)params["blob"]["maxThreshold"];
    blob_par.filterByArea = (int)params["blob"]["filterByArea"];
    blob_par.minArea = (int)params["blob"]["minArea"];
    blob_par.filterByCircularity = (int)params["blob"]["filterByCircularity"];
    blob_par.minCircularity = (double)params["blob"]["minCircularity"];
    cv::Ptr<cv::SimpleBlobDetector> _blob_detector = cv::SimpleBlobDetector::create(blob_par);

    // Read frame
    cv::Mat im;

    // Extract data from frame
    int height = im.size().height;
    cv::Matx23f _cam2world_tf = cv::Matx23f(_pixel2meter, 0, 0, 0, -_pixel2meter, _pixel2meter*height);
    cv::Matx23f _world2cam_tf = cv::Matx23f(_meter2pixel, 0, 0, 0, -_meter2pixel, height);

    // Get a clean frame (clean buffer by reading four times before keeping the final frame)
    for (int i = 0; i < 5; ++i) {
        cam.read(im);
    }

    // Remove background
    std::vector<std::vector<cv::Point> > contours;
    cv::Mat mask_copy;
    cv::Mat cont_mask;
    cv::absdiff(background, im, cont_mask);
    cv::cvtColor(cont_mask, cont_mask, CV_RGB2GRAY);
    cv::threshold(cont_mask, cont_mask, _th_bg_subtraction, 255, cv::THRESH_BINARY);
    cont_mask.copyTo(mask_copy);
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(mask_copy, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Show image
    cv::Mat debug_im = cv::Mat::zeros( im.size(), CV_8UC3 );
    for( int i = 0; i< contours.size(); i++ )
    {
        cv::Scalar color = cv::Scalar( 0, 225, 0 );
        drawContours( debug_im, contours, i, color, 2, 8, hierarchy, 0, cv::Point() );
    }

    imshow("Contours", debug_im);
    cv::waitKey(2000);
    imshow("Mask", mask_copy);
    cv::waitKey(2000);

    // Wait for user input [ENTER]
    while( !kbhit() ){
        cv::waitKey(1);
    }

    cam.stop();
}
