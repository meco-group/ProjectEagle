#ifndef STREAMER_H
#define STREAMER_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <arpa/inet.h>

class Streamer
{
    private:
        int _socket;
        struct sockaddr_in _sa;

    public:
        Streamer(const char* address, int port);

        int start();
        int send(cv::Mat &img, int quality = 20);

};

#endif //STREAMER_H
