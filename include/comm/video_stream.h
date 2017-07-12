//
// Created by peter on 12/07/17.
//

#ifndef PROJECTEAGLE_VIDEO_STREAM_H
#define PROJECTEAGLE_VIDEO_STREAM_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <iostream>

using namespace std;

class VideoStream {
public:
    VideoStream(string address, int frame_size);
};


#endif //PROJECTEAGLE_VIDEO_STREAM_H
