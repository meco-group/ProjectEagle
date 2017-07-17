#include "latitude_camera.h"

using namespace cam;


LatitudeCamera::LatitudeCamera(int device) :
    V4L2Camera(device) {
    format(640, 480, V4L2_PIX_FMT_YUYV);
	buffers(4);
	setBrightness(7);
}

int LatitudeCamera::process_buffer(cv::Mat &img) {
	readYUYV(img);
	return 0;
}

bool LatitudeCamera::readYUYV(cv::Mat &img) {
	img.create(getHeight(),getWidth(),CV_8UC3);
	cv::Mat yuyv(getHeight(),getWidth(),CV_8UC2, (char*)getBuffer());
	cv::cvtColor(yuyv,img,cv::COLOR_YUV2BGR_YUYV);
}

bool LatitudeCamera::setBrightness(int brightness) {
	if (brightness < 0 || brightness > 40){
		std::cout << "Brighness should lie between 0 and 40!" << std::endl;
		return false;
	}

	return v4l2_set_brightness(brightness) == 0;
}
