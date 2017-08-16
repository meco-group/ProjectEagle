#include "libcam.hpp"

namespace eagle {
    V4L2Camera* getCamera(int index, CamType type) {
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
}
