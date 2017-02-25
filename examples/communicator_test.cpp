#include "communicator.h"

int main(void)
{
    Communicator com("host1", "wlan0");
    com.debug();
    com.start();
    com.join("eagle");

    int data = 35;
    com.shout("header", &data, sizeof(data), "eagle");

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
    com.leave("eagle");
    com.stop();
}
