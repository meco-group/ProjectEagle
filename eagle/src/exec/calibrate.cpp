#include "calibration_settings.h"
#include "calibrator.h"

int main(int argc, char* argv[]) {
    const string settingPath = argc > 1 ? argv[1] : "/home/peter/Documents/Honours/ProjectEagle/src/client/config/devices/Pi/cal_conf.xml";
    cout << "Running calibration test for config: " << settingPath << '\n';

    // Calibrate
    Calibrator cal(settingPath);
    cal.execute();
    cal.saveCameraParams();
}
