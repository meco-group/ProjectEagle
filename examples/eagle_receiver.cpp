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

    while( k < 200 ) {
        std::vector< size_t > sizes;
        std::vector< void* > data;
        int n_obs = 0;
		std::cout << "before listen	" << std::endl;		
        if( com.listen(data, sizes, peers[0],1) ) {
			std::cout << "packet received" << std::endl;
            if (data.size()%2 != 0) {
                std::cout << "Expected even number of data packets: header + msg" << std::endl;
            } else {
                for (int k=0; k<data.size(); k+=2) {
                    eagle::header_t header;
                    header = *((eagle::header_t*)(data[k]));
                    switch( header.id ) {
                    case eagle::MARKER:
                        eagle::marker_t marker;
                        marker = *((eagle::marker_t*)(data[k+1]));
                        std::cout << "Robot " << marker.id << " detected at (" << marker.x << "," << marker.y << "," << marker.t << ")" << std::endl;
                        break;
                    case eagle::OBSTACLE:
                        n_obs++;
                        break;

					case eagle::IMAGE:
						cv::Mat rawData = cv::Mat( 1, sizes[k+1], CV_8UC1, data[k+1]);
            			cv::Mat im = cv::imdecode(rawData, 1);
						imshow("Stream",im);
            			cv::waitKey(1);
						break;
                    }
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
