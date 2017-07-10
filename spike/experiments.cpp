
#include "experiments.h"

int main(int argc, char* argv[]) {
    // Parse arguments
    const int mode = argc > 1 ? atoi(argv[1]) : 0;
    const string config1 = argc > 2 ? argv[1] : "../config/ceil1_cam.xml";
    const string config2 = argc > 3 ? argv[1] : "../config/ceil2_cam.xml";
    switch (mode) {
        case 0:
            cal_test(config1);
            break;
        case 1 :
            stereo_cal(config1, config2);
            break;
        case 2:
            detect_pattern(config1);
            break;
        default:
            break;
    }
}
