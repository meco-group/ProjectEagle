#include "cam/libcam.h"
#include "examples_config.h"
#include <opencv2/opencv.hpp>

int main(void) {
    EXAMPLE_CAMERA_T cam(EXAMPLE_CAMERA_INDEX);
    cam.start();
    cv::Mat im;

	// take a snapshot
    cam.read(im);

    cv::imwrite("snapshot.png",im);
    // imshow("snapshot",im);
    // cv::waitKey(2000);

    cam.stop();
}

