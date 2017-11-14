#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include "camera_interface.h"
#include "v4l2_camera.h"
#include "see3_camera.h"
#include "opi_camera.h"
#include "pi_camera.h"
#include "latitude_camera.h"
#include "odroid_camera.h"

namespace eagle {

enum CamType {
    INVALID, PICAM, OPICAM, SEE3CAM, LATCAM, OCAM
};

CamType getCamType(std::string camType) {
    CamType result = INVALID;
    if (!camType.compare("PICAM")) result = PICAM;
    if (!camType.compare("OPICAM")) result = OPICAM;
    if (!camType.compare("SEE3CAM")) result = SEE3CAM;
    if (!camType.compare("LATCAM")) result = LATCAM;
    if (!camType.compare("OCAM")) result = OCAM;
    return result;
}

std::string getCamType(CamType camType) {
    const char *camStrings[] = {"INVALID", "PICAM", "OPICAM", "SEE3CAM", "LATCAM", "OCAM"};
    return std::string(camStrings[camType]);
}

Camera* getCamera(int index, CamType type) {
    switch (type) {
    case PICAM:
        return new PiCamera(index);
    case OPICAM:
        return new OPICamera(index);
    case SEE3CAM:
        return new See3Camera(index);
    case LATCAM:
        return new LatitudeCamera(index);
    case OCAM:
        return new OdroidCamera(index);
    default:
        return new LatitudeCamera(index);
    }
}

Camera* getCamera(int index, std::string type) {
    return getCamera(index, getCamType(type));
}

Camera* getCamera(std::string config_path) {
    cv::FileStorage fs(config_path, cv::FileStorage::READ);
    Camera* cam = getCamera(fs["camera"]["index"], fs["camera"]["type"]);
    cam->setResolution(fs["camera"]["resolution"]["width"],
                       fs["camera"]["resolution"]["height"]);
    cv::Mat camera_matrix, distortion_vector;
    fs.release();
    return cam;
}

};

#endif //CAMERA_HPP
