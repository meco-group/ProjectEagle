#include "odroid_camera.h"

using namespace eagle;

OdroidCamera::OdroidCamera(int device) : V4L2Camera(device) {
    format(640, 480, V4L2_PIX_FMT_YUYV);
    buffers(1);
    setBrightness(2);
}

int OdroidCamera::process_buffer(cv::Mat &img) {
    readYUYV(img);
    return 0;
}

bool OdroidCamera::readYUYV(cv::Mat &img) {
    img.create(getHeight(), getWidth(), CV_8UC3);
    cv::Mat yuyv(getHeight(), getWidth(), CV_8UC2, (char*)getBuffer());
    cv::cvtColor(yuyv, img, cv::COLOR_YUV2BGR_YUYV);
}

bool OdroidCamera::setBrightness(int brightness) {
    if (brightness < -5 || brightness > 5) {
        std::cout << "Brighness should lie between 0 and 8!" << std::endl;
        return false;
    }
    return v4l2_set_brightness(brightness) == 0;
}

bool OdroidCamera::setResolution(const std::vector<int>& resolution) {
    format(resolution[0], resolution[1], V4L2_PIX_FMT_YUYV);
    return true;
}
