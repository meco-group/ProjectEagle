#ifndef EXAMPLES_CONFIG_H
#define EXAMPLES_CONFIG_H

#define EXAMPLE_CAMERA_T LatitudeCamera
#define EXAMPLE_CAMERA_INDEX 0
#define EXAMPLE_CAMERA_CALIBRATION "../config/see3cam.yml"
//#define EXAMPLE_COMMUNICATOR_INTERFACE "enp0s25"
//#define EXAMPLE_COMMUNICATOR_INTERFACE "wlp12s0"
#define EXAMPLE_COMMUNICATOR_INTERFACE "wlp58s0"

// Break from while loop: should move to separate file..
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>

int kbhit()
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

#endif //EXAMPLES_CONFIG_H
