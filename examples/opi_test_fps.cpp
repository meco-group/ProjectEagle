#include "opi_camera.h"
#include <ctime>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

int main(void)
{
	int nof = 100;

    OPICamera cam;
	cam.start();
    cv::Mat im;

	// Chrono
	time_t begin, end;
    std::cout << "Chrono: start!" << std::endl;
	time(&begin);
	
	// Start loop
    for(int k=0;k<nof;k++){
        cam.read(im);

//        std::vector<int> compression_params;
//        compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
//        compression_params.push_back(95);
//    
//        std::vector<uchar>buffer;
//        //buffer.reserve()
//        bool succes = cv::imencode(".jpg",im,buffer,compression_params);
    }
    
    // Stop chrono
    time(&end);
    std::cout << "Chrono: stop!" << std::endl;
    int duration = difftime(end,begin);
    double fps = nof/(double)duration;
    
    // print results
    std::cout << "Duration:" << duration << "[s]" << std::endl;
	std::cout << "FPS:" << fps << std::endl;

    cam.stop();
}
