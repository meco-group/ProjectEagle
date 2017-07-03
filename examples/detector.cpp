#include "libcam.h"
#include "examples_config.h"
#include "detector.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

int main(void) {
    EXAMPLE_CAMERA_T cam(EXAMPLE_CAMERA_INDEX);
//    cam.setBrightness(7);
//    cam.calibrate("../config/see3cam.yml"); //camera can be calibrated
    cam.start();

    // create detector
    Detector detector("../config/detector.yml", "background.png");

    // startte detecting
    cv::Mat im;
    cv::namedWindow("image", cv::WINDOW_AUTOSIZE);

    Robot sample(0, 0.55, 0.4, cv::Scalar(50, 50, 50));
    Robot dave(9, 0.55, 0.4, cv::Scalar(17, 110, 138));
    Robot kurt(1, 0.55, 0.4, cv::Scalar(138, 31, 17));
    std::vector< Robot* > robots = std::vector< Robot* >{&dave, &kurt, &sample};
    std::vector< Obstacle* > obstacles;

    while (true) {
        cam.read(im);
        detector.search(im, robots, obstacles);
        detector.draw(im, robots, obstacles);		
		for(int k=0; k<robots.size(); k++){
			if(robots[k]->detected()){
				std::cout << "Robot " << robots[k]->code() << " detected" << std::endl;
			}
		}
        imshow("image", im);
        cv::waitKey(10);
    }

    cam.stop();
}
