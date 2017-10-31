#include <eagle_cmd_helper.h>

using namespace eagle;

int main(int argc, char* argv[]) {

    std::string iface = (argc > 1) ? argv[1] : "wlan0";
    std::string shout_or_whisper = (argc > 2) ? argv[2] : "-g";
    std::string group_or_peer = (argc > 3) ? argv[3] : "EAGLE";
    int port = (argc > 4) ? std::stod(argv[4]) : 5670;

    // set command
    cmd_t cmd = SNAPSHOT;

    if(!send_command(cmd, iface, port, shout_or_whisper, group_or_peer)) {
        std::cout << "Transmission not succesful" << std::endl;
    }

    return 0;
}

