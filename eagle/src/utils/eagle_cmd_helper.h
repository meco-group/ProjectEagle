#ifndef EAGLE_CMD_HELPER_H
#define EAGLE_CMD_HELPER_H

#include "../communicator/communicator.h"
#include "protocol.h"
#include <unistd.h>

namespace eagle {

bool send_command(cmd_t cmd, const std::string& iface, const int port, const std::string& shout_or_whisper, const std::string& group_or_peer) {
    Communicator com("commander", iface, port);
    com.start(100);
    usleep(1000000);

    header_t header = {CMD, 0}; //set time
    // set peer
    bool succes;
    if (strcmp(shout_or_whisper.c_str(), "-g") == 0) {
        succes = com.shout(&header, &cmd, sizeof(header), sizeof(cmd), group_or_peer);
    } else if (strcmp(shout_or_whisper.c_str(), "-p") == 0) {
        succes = com.whisper(&header, &cmd, sizeof(header), sizeof(cmd), group_or_peer);
    } else {
        perror("Input error: for shout, pass -g, for whisper, pass -p");
    }
    for(int i=0; i<100000000; i++);
    com.stop();
    return succes;
}

};

#endif // EAGLE_CMD_HELPER_H
