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
       << "Camera_Resolution" << '"' << res_width << " x " << res_height << '"'
       << "Stream_Resolution" << '"' << comp_res_width << " x " << comp_res_height << '"'
       << "Camera_Calibration" << calPath
       << "}";
}

void CameraSettings::read(const FileNode &node) {
    node["Camera_Index" ] >> camIndex;
    node["Camera_Calibration"]  >> calPath;

    string resString;
    node["Camera_Resolution"] >> resString;
    resString.erase(std::remove(resString.begin(), resString.end(), ' '), resString.end());
    std::replace(resString.begin(), resString.end(), 'x', ' ');

    vector<int> resolutions;
    stringstream ss(resString);
    int temp;
    while (ss >> temp)
        resolutions.push_back(temp);

    if (resolutions.size() != 2) {
        cerr << "Invalid resolution "<< resString << '\n';
        return;
    }
    res_width = resolutions[0];
    res_height = resolutions[1];

    string compResString;
    node["Stream_Resolution"] >> compResString;
    compResString.erase(std::remove(compResString.begin(), compResString.end(), ' '), compResString.end());
    std::replace(compResString.begin(), compResString.end(), 'x', ' ');

    vector<int> comp_resolutions;
    stringstream ss2(compResString);
    int temp2;
    while (ss2 >> temp2)
        comp_resolutions.push_back(temp2);

    if (comp_resolutions.size() != 2) {
        cerr << "Invalid resolution "<< compResString << '\n';
        return;
    }
    comp_res_width = comp_resolutions[0];
    comp_res_height = comp_resolutions[1];

    string camTypeString;
    node["Camera_Type"] >> camTypeString;
    camType = getCamType(camTypeString);

    parse();
}

bool CameraSettings::parse() {
    return true;
}
