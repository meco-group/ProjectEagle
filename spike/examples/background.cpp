#include "cam/libcam.h"
#include "examples_config.h"
#include <opencv2/opencv.hpp>

int main(void) {
    EXAMPLE_CAMERA_T cam(EXAMPLE_CAMERA_INDEX);
    cam.calibrate(EXAMPLE_CAMERA_CALIBRATION);
    cam.start();

    // capture and save background
    cv::Mat frame, background;
    cam.read(frame);
    background = cv::Mat(frame.size(), CV_32FC3, cv::Scalar(0,0,0));
    for (int i=0; i<50; i++) {
        cam.read(frame);
        cv::accumulate(frame, background);
    }
    background /= 50;
    background.convertTo(background, CV_8UC3);
    cv::imwrite("background.png", background);
    imshow("Background", background);
    cv::waitKey(2000);

    cam.stop();
}

