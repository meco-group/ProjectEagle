#include "cam/libcam.h"
#include "examples_config.h"
#include "comm/communicator.h"
#include "comm/protocol.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

int main(void) {
//	std::cout << "size of size_t: " << sizeof(size_t) << std::endl;

    Communicator com("receiver", EXAMPLE_COMMUNICATOR_INTERFACE);
    com.start();
    com.join("EAGLE");

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

	// Video stream setup
    int k = 0;
    std::cout << "Opening window"<< std::endl;
    cv::namedWindow("Stream");
    eagle::header_t header;
    std::string pr;

	// Start acquiring video stream
    while( !kbhit() ) {
        if (com.listen(pr, 1)) {
            while (com.available()) {
				// 1. read the header
				com.read(&header);
				// 2. read data based on the header
                if (header.id == eagle::IMAGE) {
                    size_t size = com.framesize();
                    uchar buffer[size];
                    com.read(buffer);
                    cv::Mat rawData = cv::Mat( 1, size, CV_8UC1, buffer);
                    cv::Mat im = cv::imdecode(rawData, 1);
                    imshow("Stream",im);
                    cv::waitKey(1);
                    break;
                }
			}
        }
        k++;
    }

	// Terminate everything
    cv::destroyWindow("Stream");
    com.leave("EAGLE");
    com.stop();

    return 0;
}
