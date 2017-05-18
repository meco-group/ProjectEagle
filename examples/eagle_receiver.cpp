#include "examples_config.h"
#include "communicator.h"
#include "protocol.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

int main(void)
{
    Communicator com("receive", EXAMPLE_COMMUNICATOR_INTERFACE);
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

    // main loop
    int k = 0;
	std::cout << "Opening window"<< std::endl;
    cv::namedWindow("Stream");
	eagle::header_t header;

    while( !kbhit() ) {
        int n_obs = 0;

        if (com.listen(peers[0], 1)) {
            while (com.available()) {
				// 1. read the header
				com.read(&header);
				// 2. read data based on the header
                switch (header.id) {
                    case eagle::MARKER:
                        eagle::marker_t marker;
						com.read(&marker);
                        std::cout << "Robot " << marker.id << " detected at (" << marker.x << "," << marker.y << "," << marker.t << ")" << std::endl;

                        break;
                    case eagle::OBSTACLE:
                        n_obs++;
                        break;

					case eagle::IMAGE:
						size_t size = com.framesize();
						uchar buffer[size];
						com.read(buffer);
						cv::Mat rawData = cv::Mat( 1, size, CV_8UC1, buffer);
            			cv::Mat im = cv::imdecode(rawData, 1);
						imshow("Stream",im);
            			cv::waitKey(1);
						break;
                }
                std::cout << "Number of obstacles: " << n_obs << std::endl;
            }
        }
        k++;
		std::cout << "k = " << k << std::endl;  
    }

    cv::destroyWindow("Stream");
    com.leave("EAGLE");
    com.stop();

    return 0;
}
