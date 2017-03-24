#include "latitude_camera.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

int main(void)
{
	LatitudeCamera cam(0);
	cam.start();

    cv::Mat im;
    cv::namedWindow("image",cv::WINDOW_AUTOSIZE);

    while(true){
        cam.read(im);
        imshow("image",im);
        cv::waitKey(1);
        //std::cout << "image" << std::endl;
    }
    
    cam.stop();
}