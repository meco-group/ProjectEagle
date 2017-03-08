#include "communicator.h"
#include "latitude_camera.h"
#include "protocol.h"
#include <unistd.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <ctime>

std::vector<uchar> compress(cv::Mat &img) {
    std::vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(30);
    
    std::vector<uchar> buffer(10000,0);
    cv::imencode(".jpg",img,buffer,compression_params);
    return buffer;
}

int main(void) {
    LatitudeCamera cam(0);
    cam.start();
    cv::Mat img;

    Communicator com("eagle", "eth0");
    com.start();
    com.join("EAGLE");

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

    // start capturing
    int img_id = 0;
    eagle::header_t header;
    header.id = eagle::IMAGE;
    while(com.peers().size() > 0 ) {
        header.time = img_id;
        cam.read(img);
        std::vector<uchar> buffer = compress(img);
        if (com.shout(&header, buffer.data(), sizeof(header), buffer.size(), "EAGLE")) {
            std::cout << "Sending image " << img_id << ", size: " << buffer.size() << std::endl;
        }
        img_id++;
    }

    // stop the program
    com.leave("EAGLE");
    com.stop();
}

