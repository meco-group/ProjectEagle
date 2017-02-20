#include "opi_camera.h"
#include "streamer.h"
#include <opencv2/core/core.hpp>
#include <ctime>
#include <iostream>

int main(void)
{
    OPICamera cam(0);
    cam.configure();
	cam.start();

    Streamer streamer("10.92.191.255",60000);
    streamer.start();

    cv::Mat im;
    int count = 0;

    time_t begin,end;
    time(&begin);
    while(true){
        cam.read(im);
        streamer.send(im);

        time(&end);
        count++;
        std::cout << "FPS: " << count/((double)difftime(end,begin)) << std::endl;
    }

    cam.stop();
}
