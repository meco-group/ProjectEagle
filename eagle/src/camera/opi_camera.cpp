#include "opi_camera.h"

using namespace eagle;

OPICamera::OPICamera(int device) : V4L2Camera(device) {
    format(640, 480, V4L2_PIX_FMT_UYVY);
    buffers(1);
}

int OPICamera::process_buffer(cv::Mat &img) {
    readUYVY(img);
    return 0;
}

bool OPICamera::readUYVY(cv::Mat &img) {
    img.create(getHeight(), getWidth(), CV_8UC3);
    cv::Mat uyvy(getHeight(), getWidth(), CV_8UC2, (char*)getBuffer());
    cv::cvtColor(uyvy, img, cv::COLOR_YUV2BGR_UYVY);
}

bool OPICamera::setResolution(const std::vector<int>& resolution) {
    format(resolution[0], resolution[1], V4L2_PIX_FMT_Y16);
    return true;
}
