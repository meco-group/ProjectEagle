#include "latitude_camera.h"

LatitudeCamera::LatitudeCamera(int device) :
    V4L2Camera(device)
{
    format(640, 480, V4L2_PIX_FMT_YUYV);
	buffers(4);
}

int LatitudeCamera::process_buffer(cv::Mat &img)
{
	readYUYV(img);
	return 0;
}

bool LatitudeCamera::readYUYV(cv::Mat &img)
{
	img.create(getHeight(),getWidth(),CV_8UC3);
	cv::Mat yuyv(getHeight(),getWidth(),CV_8UC2, (char*)getBuffer());
	cv::cvtColor(yuyv,img,cv::COLOR_YUV2BGR_YUYV);
}
