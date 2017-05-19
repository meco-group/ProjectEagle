#include "libcam.h"
#include "examples_config.h"
#include "detector.h"
#include "communicator.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

int main(void)
{
    // setup the camera
    EXAMPLE_CAMERA_T cam(EXAMPLE_CAMERA_INDEX);
    cam.start();

    // setup the communication
    Communicator com("eagle", EXAMPLE_COMMUNICATOR_INTERFACE);
    com.start();
    com.join("EAGLE");

	// setup video compression 
	std::vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(30);
    std::vector<uchar> buffer(10000,0);
	
    eagle::header_t imheader;
    imheader.id = eagle::IMAGE;

    // create detector
    Detector detector("../config/detector.yml", "background.png");

    // start detecting
    cv::Mat im;

    // Make a robot which the camera should find
    Robot BB1(0, 0.55, 0.4, cv::Scalar(17, 110, 138));
    std::vector< Robot* > robots = std::vector< Robot* >{&BB1};
    std::vector< Obstacle* > obstacles;

    // main execution loop
    int nof = 0;
    while ( !kbhit() ) {
        // Detect the robots/obstacles
        cam.read(im); nof++;
        detector.search(im, robots, obstacles);

        // Send information to interested listeners
        int i = 0;
        int k = 0;

        // pack robots
        size_t n_detected_robots = 0;
        eagle::header_t mheaders[robots.size()];
        eagle::marker_t markers[robots.size()];
        for (k = 0; k < robots.size(); k++) {
            if( robots[k]->detected() ) {
                mheaders[n_detected_robots].id = eagle::MARKER;
                mheaders[n_detected_robots].time = 0;
                markers[n_detected_robots++] = robots[k]->serialize();
            }
        }
        // pack obstacles
        eagle::header_t oheaders[obstacles.size()];
        eagle::obstacle_t obs[obstacles.size()];
        for (k = 0; k < obstacles.size(); k++) {
            oheaders[k].id = eagle::OBSTACLE;
            oheaders[k].time = 0;
            obs[k] = obstacles[k]->serialize();
        }

        // make data and size vector
        std::vector< size_t > sizes = std::vector< size_t >(2*n_detected_robots + 2*obstacles.size(),0);
        std::vector< const void* > data = std::vector< const void* >(sizes.size());

        for (k = 0; k < n_detected_robots; k++) {
            sizes[i] = sizeof(eagle::header_t);
            data[i++] = (const void*)(&mheaders[k]);
            sizes[i] = sizeof(eagle::marker_t);
            data[i++] = (const void*)(&markers[k]);
        }

        for (k = 0; k < obstacles.size(); k++) {
            sizes[i] = sizeof(eagle::header_t);
            data[i++] = (const void*)(&oheaders[k]);
            sizes[i] = sizeof(eagle::obstacle_t);
            data[i++] = (const void*)(&obs[k]);
        }

        // send everything
        //com.shout(data, sizes, "EAGLE");

        // Draw everything to give some feedback
        detector.draw(im, robots, obstacles);
		imheader.time = k;
		cv::imencode(".jpg",im,buffer,compression_params);
        com.shout(&imheader, buffer.data(), sizeof(imheader), buffer.size(), "EAGLE");
        if (robots[0]->detected()){
			std::cout << "Robot detected!" << std::endl;
        }
		std::cout << "image " << nof << " sent" << std::endl;
    }

    // stop the program
    cam.stop();
    com.leave("EAGLE");
    com.stop();
}
