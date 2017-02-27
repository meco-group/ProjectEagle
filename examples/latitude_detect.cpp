#include "latitude_camera.h"
#include "detector.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

int main(void)
{
	LatitudeCamera cam(0);
    Detector detector;

    BallBot MECOBB1(0);
    Ourbot dave(5);
    Ourbot kurt(3);
    std::vector< Robot* > robots = std::vector< Robot* >{&MECOBB1, &dave, &kurt};
    std::vector< Obstacle > obstacles;

	cam.start();
    detector.start();

    cv::Mat im;
    cv::namedWindow("image",cv::WINDOW_AUTOSIZE);

    while(true){
        cam.read(im);
        detector.update(im, robots, obstacles);
        detector.draw(im, robots, obstacles);
        imshow("image",im);
        cv::waitKey(1);
    }
    
    cam.stop();
}

