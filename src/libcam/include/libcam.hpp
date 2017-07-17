#ifndef PROJECTEAGLE_LIBCAM_HPP
#define PROJECTEAGLE_LIBCAM_HPP

#include "../src/types/see3_camera.h"
#include "../src/types/opi_camera.h"
#include "../src/types/pi_camera.h"
#include "../src/types/latitude_camera.h"
#include "../src/types/odroid_camera.h"

namespace cam {
    using namespace std;
    using namespace cv;

    enum CamType {
        INVALID, PICAM, OPICAM, SEE3CAM, LATCAM, OCAM
    };

    V4L2Camera *getCamera(int index, CamType type);

    CamType getCamType(std::string camType);

    std::string getCamType(CamType camType);
};

#endif //PROJECTEAGLE_LIBCAM_HPP
