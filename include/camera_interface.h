#ifndef CAMERA_INTERFACE_H
#define CAMERA_INTERFACE_H

#include <chrono>
#include <opencv2/core/core.hpp>

class CameraInterface
{
private:
    

public:
	CameraInterface(int device){};
	
	virtual bool isOpened() = 0;
	virtual bool read(cv::Mat &img) = 0;
    virtual double captureTime();
    
    virtual int getWidth() = 0;
    virtual int getHeight() = 0;

};

#endif //CAMERA_INTERFACE_H
