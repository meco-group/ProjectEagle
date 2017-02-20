#include "opi_camera.h"
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

int main(void)
{
	OPICamera cam(0);
    cam.configure();
	cam.start();
	
    cv::Mat im;
	cam.read(im);
	imwrite("opipicture.jpg", im);
	std::cout << "Picture taken" << std::endl;
    
    cam.stop();
}

