#include "detector.h"
#include "see3_camera.h"

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

    // create detector
    Detector detector("../config/detector.yml", "background.png");

    // start detecting
    cv::Mat im;
    cv::namedWindow("image", cv::WINDOW_AUTOSIZE);

    Robot dave(9, 0.55, 0.4, cv::Scalar(17, 110, 138));
    Robot kurt(1, 0.55, 0.4, cv::Scalar(138, 31, 17));
    std::vector< Robot* > robots = std::vector< Robot* >{&dave, &kurt};
    std::vector< Obstacle* > obstacles;

    while (true) {
        cam.read(im);
        detector.search(im, robots, obstacles);
        detector.draw(im, robots, obstacles);
        imshow("image", im);
        cv::waitKey(10);
    }

    cam.stop();
}
