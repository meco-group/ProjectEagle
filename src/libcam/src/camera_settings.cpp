//
// Created by peter on 07/07/17.
//

#include "camera_settings.h"

using namespace cam;

CameraSettings::CameraSettings() : Config("CameraSettings") {}

void CameraSettings::write(FileStorage &fs) const {
    fs << _nodeName << "{"
       << "Camera_Index"  << camIndex
       << "Camera_Type" << getCamType(camType)
       << "Camera_Calibration" << calPath
       << "}";
}

void CameraSettings::read(const FileNode &node) {
    node["Camera_Index" ] >> camIndex;
    node["Camera_Calibration"]  >> calPath;

    string camTypeString;
    node["Camera_Type"] >> camTypeString;
    camType = getCamType(camTypeString);

    parse();
}

bool CameraSettings::parse() {
    return true;
}
