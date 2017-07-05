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

    eagle::header_t header;
    size_t hsize;
    uchar data[20000];
    size_t dsize;

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
    int nof = 0;
	double total_duration_micros = 0;
	std::chrono::system_clock::time_point begin;
	std::chrono::system_clock::time_point end;
	std::cout << "Opening window (press enter to stop the program)" << std::endl;
    cv::namedWindow("Stream");

	// Start acquiring video stream
    while( !kbhit() ) {
		begin = std::chrono::system_clock::now();
        if (com.listen(peers[0], 1)) {
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

					//timing diagnostics
					end = std::chrono::system_clock::now();
					total_duration_micros += std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
					break;
                }
			}
        }
        nof++;
    }

    // Diagnose the statistics
	double average_duration = total_duration_micros*1e-6/nof;
    std::cout << "Samples: " << nof << std::endl;
	std::cout << "Average FPS: " << 1/average_duration << std::endl;

	// Terminate everything
    cv::destroyWindow("Stream");
    com.leave("EAGLE");
    com.stop();

    return 0;
}
