//
// Created by peter on 12/07/17.
//

#include "com_settings.h"

using namespace eagle;

ComSettings::ComSettings() : Config("CommunicatorSettings") {}

void ComSettings::write(FileStorage &fs) const {
    fs << "CommunicatorSettings" << "{"
       << "Group"  << group
       << "Interface"  << interface
       << "Init_Wait_Time" << init_wait_time
       << "}";
}

void ComSettings::read(const FileNode &node) {
    node["Group"] >> group;
    node["Interface" ] >> interface;
    node["Init_Wait_Time"] >> init_wait_time;
    parse();
}

bool ComSettings::parse() {
    return true;
}
