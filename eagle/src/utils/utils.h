#ifndef UTILS_H
#define UTILS_H

#define CONFIG_PATH "/home/ruben/Documents/Work/Repositories/ProjectEagle/eagle/config/config.yml"

#include <cstdio>
#include <zconf.h>

namespace eagle {

    int kbhit() {
        struct timeval tv;
        fd_set fds;
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
        select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
        return FD_ISSET(STDIN_FILENO, &fds);
    }

}

#endif //UTILS_H

