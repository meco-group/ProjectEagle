#include "cam/libcam.h"
#include "examples_config.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

int main(void)
{
	EXAMPLE_CAMERA_T cam(EXAMPLE_CAMERA_INDEX);
	cam.start();

    cv::Mat im;
    cv::namedWindow("Viewer",cv::WINDOW_AUTOSIZE);
    std::cout << "Hit enter to stop the program." << std::endl;

    while( !kbhit() ){
        cam.read(im);
        imshow("Viewer",im);
        cv::waitKey(1);
    }
    
    cam.stop();
    return 0;
}
