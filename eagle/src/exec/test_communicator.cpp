#include <eagle.h>
#include <ctime>
#include <random>

using namespace eagle;

int main(void) {
    std::srand(std::time(NULL));
    // read config file
    cv::FileStorage fs(CONFIG_PATH, cv::FileStorage::READ);

    int nr = std::rand()%10;
    Communicator com("host" + std::to_string(nr), CONFIG_PATH);
    com.debug();
    com.start(fs["communicator"]["zyre_wait_time"]);
    com.join("EAGLE");

    char* data_snd;
    char data_rcv[1024];
    std::string header;
    std::string peer;
    size_t size_rcv;

    data_snd = "ping";
    if (com.shout("cmd", data_snd, sizeof(data_snd), "EAGLE")) {
        std::cout << "Sending " << "cmd:" << data_snd << " to EAGLE." << std::endl;
    }
    if (com.listen(header, data_rcv, size_rcv, peer, 10)) {
        std::cout << "Receiving " << header << ":" << data_rcv <<
        " from " << peer << "." << std::endl;
    }
    data_snd = "pong";
    if (com.shout("cmd", data_snd, sizeof(data_snd), "EAGLE")) {
        std::cout << "Sending " << "cmd:" << data_snd << " to EAGLE." << std::endl;
    }
    if (com.listen(header, data_rcv, size_rcv, peer, 10)) {
        std::cout << "Receiving " << header << ":" << data_rcv <<
        " from " << peer << "." << std::endl;
    }

    std::vector<std::string> groups = com.mygroups();
    std::cout << "my groups: " << std::endl;
    for (auto &group : groups) {
        std::cout << "* " << group << std::endl;
    }
    groups = com.allgroups();
    std::cout << "all groups: " << std::endl;
    for (auto &group : groups) {
        std::cout << "* " << group << std::endl;
    }
    std::vector<std::string> peers = com.peers();
    std::cout << "peers: " << std::endl;
    for (auto &peer : peers) {
        std::cout << "* " << peer << std::endl;
    }
    peers = com.peers("EAGLE");
    std::cout << "peers in EAGLE: " << std::endl;
    for (auto &peer : peers) {
        std::cout << "* " << peer << std::endl;
    }

    com.leave("EAGLE");
    com.stop();
}
