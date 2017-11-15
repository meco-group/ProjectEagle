#include <eagle.h>
#include <ctime>
#include <random>

using namespace eagle;

int main(int argc, char* argv[]) {
    std::string iface = (argc > 1) ? argv[1] : "wlan0";
    int port = (argc > 2) ? std::stoi(argv[2]) : 5670;
    int wait_time = (argc > 3) ? std::stoi(argv[3]) : 10;

    std::srand(std::time(NULL));
    // read config file
    int nr = std::rand() % 10;
    Communicator com("host" + std::to_string(nr), iface, port);
    com.verbose(2);
    com.start(wait_time);
    com.join("eagle");
    com.join("group" + std::to_string(nr));

    const char* data_snd;
    char data_rcv[1024];
    std::string header;
    std::string peer;
    size_t size_rcv;

    data_snd = "ping";
    if (com.shout("cmd", data_snd, sizeof(data_snd), "eagle")) {
        std::cout << "Sending " << "cmd:" << data_snd << " to eagle." << std::endl;
    }
    if (com.listen(header, data_rcv, size_rcv, peer, 10)) {
        std::cout << "Receiving " << header << ":" << data_rcv <<
                  " from " << peer << "." << std::endl;
    }
    data_snd = "pong";
    if (com.whisper("cmd", data_snd, sizeof(data_snd), com.peers()[0])) {
        std::cout << "Sending " << "cmd:" << data_snd << " to " << com.peers()[0] << "." << std::endl;
    }
    if (com.listen(header, data_rcv, size_rcv, peer, 10)) {
        std::cout << "Receiving " << header << ":" << data_rcv <<
                  " from " << peer << "." << std::endl;
    }

    std::vector<std::string> groups = com.mygroups();
    std::cout << "My groups" << std::endl;
    for (uint k = 0; k < groups.size(); k++) {
        std::cout << "* " << groups[k] << std::endl;
    }

    groups = com.allgroups();
    std::cout << "All groups I know about" << std::endl;
    for (uint k = 0; k < groups.size(); k++) {
        std::cout << "* " << groups[k] << std::endl;
    }

    std::vector<std::string> peers = com.peers();
    std::cout << "My peers" << std::endl;
    for (uint k = 0; k < peers.size(); k++) {
        std::cout << "* " << peers[k] << std::endl;
    }


    com.leave("eagle");
    com.leave("group"+std::to_string(nr));
    com.stop();
}
