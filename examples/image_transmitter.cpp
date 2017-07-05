#include "cam/libcam.h"
#include "examples_config.h"
#include "comm/communicator.h"
#include "comm/protocol.h"
#include <unistd.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

int main(void) {
    EXAMPLE_CAMERA_T cam(EXAMPLE_CAMERA_INDEX);
    cam.start();
    cv::Mat img;

    Communicator com("eagle", EXAMPLE_COMMUNICATOR_INTERFACE);
    com.start();
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
	
	// Loop over all peers to send images
	auto begin = std::chrono::system_clock::now();
    while(com.peers().size() > 0 ) {
        header.time = img_id;
        cam.read(img);
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

