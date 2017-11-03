#include "communicator.h"
#include "utils.h"
#include <ctime>
#include <random>

using namespace eagle;

int main(int argc, char* argv[]) {
    std::string iface = (argc > 1) ? argv[1] : "wlan0";
    int port = (argc > 2) ? std::stoi(argv[2]) : 5670;
    int wait_time = (argc > 3) ? std::stoi(argv[3]) : 10;

    std::srand(std::time(NULL));
    // read config file
    int nr = std::rand()%10;
    Communicator com("host" + std::to_string(nr), iface, port);
    com.debug();
    com.start(wait_time);
    com.join("EAGLE");
    com.join("EXTRA");
    
    std::cout << "here?" << std::endl;

    Communicator com1("host-extra", iface, port);
    com1.debug();
    com1.start(wait_time);
    com1.join("EAGLE");

    std::cout << "here? ..." << std::endl;

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

    for (uint k=0; k<10; k++) {
        com.listen(peer, 1);
        com1.listen(peer, 1);
    }

    std::vector<std::string> groups = com.mygroups();
    std::cout << "COM0 groups" << std::endl;
    for (uint k=0; k<groups.size(); k++) {
        std::cout << groups[k] << std::endl;
    }

    groups = com1.mygroups();
    std::cout << "COM1 groups" << std::endl;
    for (uint k=0; k<groups.size(); k++) {
        std::cout << "* " << groups[k] << std::endl;
    }

    groups = com1.allgroups();
    std::cout << "COM all groups" << std::endl;
    for (uint k=0; k<groups.size(); k++) {
        std::cout << "* " << groups[k] << std::endl;
    }

    std::vector<std::string> peers = com.peers();
    std::cout << "COM all peers" << std::endl;
    for (uint k=0; k<peers.size(); k++) {
        std::cout << "* " << peers[k] << std::endl;
    }

    peers = com1.peers();
    std::cout << "COM all peers" << std::endl;
    for (uint k=0; k<peers.size(); k++) {
        std::cout << "* " << peers[k] << std::endl;
    }

    data_snd = "from_com";
    com.whisper("test", data_snd, sizeof(data_snd), com1.name());
    if (!com1.listen(peer, 10)) {
        std::cout << "from_com not received" << std::endl;
    }

    com.leave("EAGLE");
    com.leave("EXTRA");
    com.stop();
    com1.leave("EAGLE");
    com1.stop();
}
