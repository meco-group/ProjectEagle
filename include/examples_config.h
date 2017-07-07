#ifndef EXAMPLES_CONFIG_H
#define EXAMPLES_CONFIG_H

#define EXAMPLE_CAMERA_T LatitudeCamera
#define EXAMPLE_CAMERA_INDEX 0
#define EXAMPLE_CAMERA_CALIBRATION "../config/see3cam.yml"
//#define EXAMPLE_COMMUNICATOR_INTERFACE "enp0s25"
//#define EXAMPLE_COMMUNICATOR_INTERFACE "wlp12s0"
//#define EXAMPLE_COMMUNICATOR_INTERFACE "wlp58s0"
#define EXAMPLE_COMMUNICATOR_INTERFACE "wlan0"

// Break from while loop: should move to separate file..
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>

int kbhit();

#endif //EXAMPLES_CONFIG_H
