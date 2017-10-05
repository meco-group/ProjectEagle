#include "calibrator.h"
#include "utils.h"

using namespace eagle;

int main(int argc, char* argv[]) {
    std::string config_path = (argc > 1) ? argv[1] : CONFIG_PATH;
    std::string images_path = (argc > 2) ? argv[2] : CAL_IMAGES_PATH;
    // calibrate
    Calibrator cal(config_path);
    cal.execute(images_path,false);
    cal.save(config_path);
    return 0;
}
