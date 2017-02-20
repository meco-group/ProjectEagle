#include "streamer.h"

Streamer::Streamer(const char* address, int port)
{
    // set AF_INET field
    _sa.sin_family = AF_INET;
    // store this IP address in sa:
    inet_pton(AF_INET, address, &(_sa.sin_addr));
    //_sa.sin_addr.s_addr = inet_addr(address);
    // store port in sa (htons for network byte order):
    _sa.sin_port = htons(port);
}

int Streamer::start()
{
    _socket = socket(AF_INET, SOCK_DGRAM, 0);
    int broadcastEnable=1;
    int ret=setsockopt(_socket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
    if(_socket < 0){
        perror("Opening socket");
        return -1;
    }

    return 0;
}

int Streamer::send(cv::Mat &img, int quality)
{
    std::vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(quality);

    std::vector<uchar>buffer;
    //buffer.reserve()
    bool succes = cv::imencode(".jpg",img,buffer,compression_params);
    int sent = sendto(_socket,buffer.data(),buffer.size(), 0,(struct sockaddr *)&_sa,sizeof(struct sockaddr_in));
    if(sent < 0){
        perror("sendto");
        return -1;
    }

    return 0;
}
