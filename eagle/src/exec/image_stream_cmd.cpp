//#include <eagle_cmd_helper.h>
#include <communicator.h>
#include <protocol.h>

using namespace eagle;

int main(int argc, char* argv[]) {

    std::string strcmd = (argc > 1) ? argv[1] : "on";
    std::string iface = (argc > 2) ? argv[2] : "wlan0";
    std::string shout_or_whisper = (argc > 3) ? argv[3] : "-g";
    std::string group_or_peer = (argc > 4) ? argv[4] : "EAGLE";

    Communicator com("commander", iface);
    header_t header = {CMD, 0};

    // set command
    cmd_t cmd;
    if (strcmp(strcmd.c_str(),"on") == 0) {
        cmd = IMAGE_STREAM_ON;
    } else if (strcmp(strcmd.c_str(),"off") == 0) {
        cmd = IMAGE_STREAM_OFF;
    } else if (strcmp(strcmd.c_str(),"toggle") == 0) {
        cmd = IMAGE_STREAM_TOGGLE;
    } else {
        perror("Input error: unknown command. Supported: on, off, toggle");
    }

    // set peer
    if (strcmp(shout_or_whisper.c_str(),"-g") == 0) {
        com.shout(&header, &cmd, sizeof(header), sizeof(cmd), group_or_peer);
    } else if (strcmp(shout_or_whisper.c_str(),"-p") == 0) {
        com.whisper(&header, &cmd, sizeof(header), sizeof(cmd), group_or_peer);
    } else {
        perror("Input error: for shout, pass -g, for whisper, pass -p");
    }

    return 0;
}
