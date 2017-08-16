#include "camera.hpp"
#include "utils.h"

using namespace eagle;

int main(int argc, char* argv[]) {
    Camera* cam = getCamera(CONFIG_PATH);
    cam->start();

    // capture and save background
    cv::Mat frame, background;
    cam->read(frame);
    background = cv::Mat(frame.size(), CV_32FC3, cv::Scalar(0,0,0));
    for (int i=0; i<50; i++) {
        cam->read(frame);
        cv::accumulate(frame, background);
    }
    background /= 50;
    background.convertTo(background, CV_8UC3);
    cv::FileStorage fs(CONFIG_PATH, cv::FileStorage::READ);
    cv::imwrite(fs["detector"]["background_path"], background);
    fs.release();
    imshow("Background", background);
    cv::waitKey(2000);

    cam->stop();
    delete cam;
    return 0;
}

