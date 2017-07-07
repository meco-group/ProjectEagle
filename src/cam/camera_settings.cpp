//
// Created by peter on 07/07/17.
//

#include "cam/camera_settings.h"

CameraSettings::CameraSettings() : goodInput(false) {}

void CameraSettings::write(FileStorage &fs) const {
    fs << "CameraSettings" << "{"
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

void CameraSettings::parse() {
    goodInput = true;
}
