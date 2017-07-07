#ifndef LIBCAM_H
#define LIBCAM_H

#include "see3_camera.h"
#include "opi_camera.h"
#include "pi_camera.h"
#include "latitude_camera.h"

enum CamType {INVALID, PICAM, OPICAM, SEE3CAM, LATCAM};

V4L2Camera *getCamera(int index, CamType type);
CamType getCamType(std::string camType);
std::string getCamType(CamType camType);

#endif //LIBCAM_H
