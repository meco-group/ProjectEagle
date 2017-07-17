//
// Created by peter on 12/07/17.
//

#include "com_settings.h"

using namespace com;

ComSettings::ComSettings() : Config("CommunicatorSettings") {}

void ComSettings::write(FileStorage &fs) const {
    fs << "CommunicatorSettings" << "{"
       << "Interface"  << interface
       << "Init_Wait_Time" << init_wait_time
       << "}";
}

void ComSettings::read(const FileNode &node) {
    node["Interface" ] >> interface;
    node["Init_Wait_Time"] >> init_wait_time;
    parse();
}

bool ComSettings::parse() {
    return true;
}
