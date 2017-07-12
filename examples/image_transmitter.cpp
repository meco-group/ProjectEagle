#include "cam/libcam.h"
#include "examples_config.h"
#include "comm/communicator.h"
#include "comm/protocol.h"
#include <unistd.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <comm/comm_settings.h>
#include <cam/camera_settings.h>

int main(int argc, char* argv[]) {
    // Parse arguments
    const string config = argc > 1 ? argv[1] : "../config/ceil1_cam.xml";

    FileStorage fs(config, FileStorage::READ);
    CameraSettings cameraSettings;
    fs["CameraSettings"] >> cameraSettings;
    CommSettings commSettings;
    fs["CommunicatorSettings"] >> commSettings;
    fs.release();

    // EXAMPLE_CAMERA_T cam(EXAMPLE_CAMERA_INDEX);
    V4L2Camera *cam = getCamera(cameraSettings.camIndex, cameraSettings.camType);

    // Start camera
    // cam->calibrate(cameraSettings.calPath); //camera can be calibrated
    cam->start();

    Communicator com("eagle", commSettings.interface);
    com.start(commSettings.init_wait_time);
    com.join("EAGLE");

	std::vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(30);
    std::vector<uchar> buffer(10000,0);

    // wait for peer
    std::cout << "waiting for peers" << std::endl;
    while(com.peers().size() <= 0) {
        sleep(1);
    }

    // print peers
    std::vector<std::string> peers = com.peers();
    std::cout << "peers: " << std::endl;
    for (auto &peer : peers) {
        std::cout << "* " << peer << std::endl;
    }

    // start capturing
    std::cout << "Start capturing video stream." << std::endl;
    
	int img_id = 0;
    eagle::header_t header;
    header.id = eagle::IMAGE;

    Mat img;
	
	// Loop over all peers to send images_ceil1
	auto begin = std::chrono::system_clock::now();
    while(com.peers().size() > 0 ) {
        header.time = img_id;
        cam->read(img);
        cv::imencode(".jpg",img,buffer,compression_params);
        if (com.shout(&header, buffer.data(), sizeof(header), buffer.size(), "EAGLE")) {
            std::cout << "Sending image " << img_id << ", size: " << buffer.size() << std::endl;
        }
        img_id++;
    }

	// Stop the loop: no peers connected
	auto end = std::chrono::system_clock::now();
	double fps = img_id/(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()*1e-6);
	std::cout << "Average framerate: " << fps << std::endl;

    // stop the program
    com.leave("EAGLE");
    com.stop();
}

