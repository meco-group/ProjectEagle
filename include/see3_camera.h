#ifndef SEE3_CAMERA_H
#define SEE3_CAMERA_H

#include "v4l2_camera.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <iostream>

class See3Camera : public V4L2Camera
{
private:
    int process_buffer(cv::Mat &img);
    bool readBayer(cv::Mat &img);
    bool readBayerIR(cv::Mat &img);

public:
    See3Camera(int device = 0);

    bool setResolution(const std::vector<int>& resolution);
    bool setBrightness(int brightness);
    bool setExposure(int exposure);
    bool setISO(int iso);

};

#endif //SEE3_CAMERA_H
