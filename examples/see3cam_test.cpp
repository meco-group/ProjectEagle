#include "see3_camera.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

int main(void)
{
	See3Camera cam(1);
	cam.format(1280,720,V4L2_PIX_FMT_Y16);
	cam.configure();
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
