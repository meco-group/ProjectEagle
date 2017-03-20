#include "see3_camera.h"
#include <opencv2/opencv.hpp>

int main(void) {
    See3Camera cam(1);
    cam.setBrightness(7);
    cam.calibrate("../config/see3cam.yml");
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
    imshow("image", background);
    cv::waitKey(2000);

    cam.stop();
}

