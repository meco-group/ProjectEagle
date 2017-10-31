#include <eagle_cmd_helper.h>

using namespace eagle;

int main(int argc, char* argv[]) {

    std::string strcmd = (argc > 1) ? argv[1] : "on";
    std::string iface = (argc > 2) ? argv[2] : "wlan0";
    std::string shout_or_whisper = (argc > 3) ? argv[3] : "-g";
    std::string group_or_peer = (argc > 4) ? argv[4] : "EAGLE";
    int port = (argc > 5) ? std::stod(argv[5]) : 5670;

    // set command
    cmd_t cmd;
    if (strcmp(strcmd.c_str(),"on") == 0) {
        cmd = CALIBRATION_ON;
    } else if (strcmp(strcmd.c_str(),"off") == 0) {
        cmd = CALIBRATION_OFF;
    } else if (strcmp(strcmd.c_str(),"toggle") == 0) {
        cmd = CALIBRATION_TOGGLE;
    } else {
        perror("Input error: unknown command. Supported: on, off, toggle");
    }

    if(!send_command(cmd, iface, port, shout_or_whisper, group_or_peer)) {
        std::cout << "Transmission not succesful" << std::endl;
    }

    return 0;
}

