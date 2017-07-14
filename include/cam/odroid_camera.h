#ifndef ODRDOID_CAMERA_H
#define ODRDOID_CAMERA_H

#include "v4l2_camera.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

class OdroidCamera : public V4L2Camera {
private:
	int process_buffer(cv::Mat &img);    
	bool readYUYV(cv::Mat &img);

public:
	OdroidCamera(int device = 0);
	bool setBrightness(int brightness);

};

#endif //ODRDOID_CAMERA_H
