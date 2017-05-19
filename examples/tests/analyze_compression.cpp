#include "libcam.h"
#include "examples_config.h"
#include <ctime>
#include <chrono>
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

	// Compression settings and buffer
    std::vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(25);
    std::vector<uchar>buffer;
    //buffer.reserve()

	/* 
	 * TEST 1: Check framerate with compression active
	 */

	std::cout << std::endl << "Test 1: Checking FPS with jpg-compression." << std::endl;	
	// Chrono
	time_t begin, end;
	time(&begin);
	
	// Start capturing and compressing
    do {
        cam.read(im);
        cv::imencode(".jpg",im,buffer,compression_params);
		nof++;
		time(&end);
	} while(difftime(end,begin) < dt);

    // Stop chrono
    int duration = difftime(end,begin);
    double fps = nof/(double)duration;
    
    // print results
    std::cout << "Duration: " << duration << "s" << std::endl;
	std::cout << "FPS: " << fps << std::endl;

	/* 
	 * TEST 2: Measure average compression time
	 */
	std::cout << std::endl << "Test 2: Checking jpg-compression average duration." << std::endl;	

	double total_duration_micros = 0;
	for(int k = 0;k<nof;k++){
        cam.read(im);
		auto begin2 = std::chrono::system_clock::now();
        cv::imencode(".jpg",im,buffer,compression_params);
		auto end2 = std::chrono::system_clock::now();
		total_duration_micros += std::chrono::duration_cast<std::chrono::microseconds>(end2 - begin2).count();
	}

	double average_duration = total_duration_micros*1e-6/nof;

    // print results
    std::cout << "Samples: " << nof << std::endl;
	std::cout << "Average duration: " << average_duration << "s" << std::endl;
	std::cout << "Compression/Total: " << fps*average_duration << std::endl;

    cam.stop();
}
