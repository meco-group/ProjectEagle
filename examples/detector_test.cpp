#include "detector.h"
#include "latitude_camera.h"


int main(void) {
    LatitudeCamera cam(0);

    // capture background
    cv::Mat frame, background;
    cam.read(frame);
    background = cv::Mat(frame.size(), CV_32FC3, cv::Scalar(0,0,0));
    for (int i=0; i<25; i++) {
        cam.read(frame);
        cv::accumulate(frame, background);
    }
    background /= 25;
    background.convertTo(background, CV_8UC3);
    // create detector
    Detector detector("detector_params.yml", background);
}
