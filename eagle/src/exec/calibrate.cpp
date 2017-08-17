#include "calibration_settings.h"
#include "calibrator.h"
#include "utils.h"

using namespace eagle;

int main(int argc, char* argv[]) {
    // calibrate
    Calibrator cal(CONFIG_PATH);
    cal.execute();
    cal.saveCameraParams();
    return 0;
}
