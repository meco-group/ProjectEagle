#include "camera_interface.h"

double CameraInterface::captureTime()
{
    uint32_t ms;
    double s;
    ms = std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch()).count();
    s = ms * double(std::chrono::milliseconds::period::num) / std::chrono::milliseconds::period::den;

    return s;
}
