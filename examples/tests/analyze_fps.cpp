#include "libcam.h"
#include "examples_config.h"
#include <ctime>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

int main(void)
{
	int nof = 0;
	int dt = 10; //average over 10 seconds

    EXAMPLE_CAMERA_T cam(EXAMPLE_CAMERA_INDEX);
	cam.start();
    cv::Mat im;

	// Chrono
	time_t begin, end;
    std::cout << "Chrono: start!" << std::endl;
	time(&begin);
	
	// Read as many frames as possible
    do {
        cam.read(im);
		nof++;	
		time(&end);
	} while(difftime(end,begin) < dt);

    // Stop chrono
    std::cout << "Chrono: stop!" << std::endl;
    int duration = difftime(end,begin);
    double fps = nof/(double)duration;
    
    // print results
    std::cout << "Duration:" << duration << " [s]" << std::endl;
	std::cout << "FPS:" << fps << std::endl;

    cam.stop();
}
