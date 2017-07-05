#ifndef PI_CAMERA_H
#define PI_CAMERA_H

#include "v4l2_camera.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

class PiCamera : public V4L2Camera
{
private:
	int process_buffer(cv::Mat &img);    
	bool readYUYV(cv::Mat &img);

public:
    LatitudeCamera(int device = 0);
	bool setBrightness(int brightness);

};

#endif //PI_CAMERA_H
