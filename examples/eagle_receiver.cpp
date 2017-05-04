#include "communicator.h"
#include "protocol.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

int main(void)
{
    Communicator com("eagle", "wlan0");
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

    // main loop
    int k = 0;

    while( k < 200 ) {
        std::vector< size_t > sizes = {sizeof(eagle::header_t), 0};
        std::vector< void* > data(2);
        data[0] = malloc(sizeof(eagle::header_t));
        data[1] = malloc(1024);
        int n_obs = 0;
        if (com.listen(peers[0], 1)) {
            while (com.available()) {
                com.read(2, data, sizes);
                eagle::header_t header = *((eagle::header_t*)(data[0]));
                switch (header.id) {
                    case eagle::MARKER:
                        eagle::marker_t marker;
                        marker = *((eagle::marker_t*)(data[1]));
                        std::cout << "Robot " << marker.id << " detected at (" << marker.x << "," << marker.y << "," << marker.t << ")" << std::endl;
                        break;
                    case eagle::OBSTACLE:
                        n_obs++;
                        break;
                }
                std::cout << "Number of obstacles: " << n_obs << std::endl;
            }
        }
        k++;
    }

    com.leave("EAGLE");
    com.stop();

    return 0;
}
