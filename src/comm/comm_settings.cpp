//
// Created by peter on 12/07/17.
//

#include "comm/comm_settings.h"

CommSettings::CommSettings() : goodInput(false) {}

void CommSettings::write(FileStorage &fs) const {
    fs << "CommunicatorSettings" << "{"
       << "Interface"  << interface
       << "Init_Wait_Time" << init_wait_time
       << "}";
}

void CommSettings::read(const FileNode &node) {
    node["Interface" ] >> interface;
    node["Init_Wait_Time"] >> init_wait_time;
    parse();
}

void CommSettings::parse() {
    goodInput = true;
}
