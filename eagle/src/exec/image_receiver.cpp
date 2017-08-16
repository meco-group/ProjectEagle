#include "communicator.h"
#include "utils.h"
#include "protocol.h"

using namespace eagle;

int main(int argc, char* argv[]) {
    // Parse arguments
    std::string iface = (argc > 1) ? argv[1] : "wlan0";
    std::string group = (argc > 2) ? argv[2] : "EAGLE";
    std::string peer = (argc > 3) ? argv[3] : "eagle0";

    Communicator com("receiver", iface, 5670);
    com.start(100.);
    com.join(group);

    // wait for peer
    std::cout << "waiting for peers" << std::endl;
    while(com.peers().size() <= 0) {
        sleep(1);
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
                    if (pr == peer) {
                        cv::Mat rawData = cv::Mat( 1, size, CV_8UC1, buffer);
                        cv::Mat im = cv::imdecode(rawData, 1);
                        imshow("Stream",im);
                        cv::waitKey(1);
                    }
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
