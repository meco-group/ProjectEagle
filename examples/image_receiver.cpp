#include "libcam.h"
#include "examples_config.h"
#include "communicator.h"
#include "protocol.h"
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
    int k = 0; int nof = 200;
	double total_duration_micros = 0;
	std::chrono::system_clock::time_point begin;
	std::chrono::system_clock::time_point end;
	std::cout << "Opening window"<< std::endl;
    cv::namedWindow("Stream");

	// Start acquiring video stream
    while( k < nof ) {
		begin = std::chrono::system_clock::now();
        if( com.listen(&header, data, hsize, dsize, peers[0],1) ) {
            cv::Mat rawData  =  cv::Mat( 1, dsize, CV_8UC1, data);
            cv::Mat im = cv::imdecode(rawData, 1);
			end = std::chrono::system_clock::now();
			total_duration_micros += std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
            imshow("Stream",im);
            cv::waitKey(1);
        }
        k++;
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
