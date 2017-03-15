#include "communicator.h"
#include "protocol.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

int main(void) {
    Communicator com("eagle", "wlan0");
    com.start();
    com.join("EAGLE");

    eagle::header_t header;
    size_t hsize;
    uchar data[30000];
    size_t dsize;

    // wait for peer
    std::cout << "waiting for peers";
    while(com.peers().size() <= 0) {
        sleep(1);
        std::cout << ".";
    }
    std::cout << std::endl;

    // print peers
    std::vector<std::string> peers = com.peers();
    std::cout << "peers: " << std::endl;
    for (auto &peer : peers) {
        std::cout << "* " << peer << std::endl;
    }

    int k = 0;
    cv::namedWindow("image");
    while( k < 200 ) {
        if( com.listen((void*)&header, data, hsize, dsize, peers[0],1) ) {
            cv::Mat rawData  =  cv::Mat( 1, dsize, CV_8UC1, data);
            cv::Mat im = cv::imdecode(rawData, 1);
            imshow("image",im);
            cv::waitKey(1);
        }
        k++;
    }

    cv::destroyWindow("image");
    com.leave("EAGLE");
    com.stop();

    return 0;
}
