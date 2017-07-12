
#include "experiments.h"

int main(int argc, char* argv[]) {
    string str = "transmit";
    // Parse arguments
    const int mode = argc > 1 ? atoi(argv[1]) : 2;
    const string config1 = argc > 2 ? argv[2] : "../config/ceil1_cam.xml";
    const string config2 = argc > 3 ? argv[3] : "../config/ceil2_cam.xml";
    const bool transmit = argc > 3 ? (!str.compare(argv[3])) : false;

    switch (mode) {
        case 0:
            cal_test(config1);
            break;
        case 1 :
            stereo_cal(config1, config2);
            break;
        case 2:
            detect_pattern(config1, transmit);
            break;
        default:
            break;
    }
}
