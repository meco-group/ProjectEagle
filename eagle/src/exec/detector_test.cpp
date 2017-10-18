#include "detector.h"
#include "utils.h"

using namespace eagle;

int main(int argc, char* argv[]) {
    // robots and obstacles that the detector should search for
    Robot dave(0, 0.55, 0.4, cv::Scalar(138, 110, 17));
    Robot krist(1, 0.55, 0.4, cv::Scalar(17, 31, 138));
    Robot kurt(9, 0.55, 0.4, cv::Scalar(19, 138, 17));
    std::vector< Robot* > robots = std::vector< Robot* >{&dave, &krist, &kurt};
    std::vector< Obstacle* > obstacles;
    cv::Mat image, bg;
    image = cv::imread(argv[1], CV_LOAD_IMAGE_COLOR);
    bg = cv::imread(argv[2], CV_LOAD_IMAGE_COLOR);
    Detector detector(argv[3],bg);
    detector.search(image, robots, obstacles);
    cv::Mat undist = detector.draw(image, robots, obstacles);
    
    for (uint k=0; k<robots.size(); k++) {
        if (robots[k]->detected()) {
            std::cout << "Robot: " << robots[k]->id() << std::endl;
            std::cout << "Position: " <<robots[k]->translation() << std::endl << "Roll-pitch-yaw: " << robots[k]->rotation() << std::endl;
        }
    }
    for (uint k=0; k<obstacles.size(); k++) {
        cloud3_t p = obstacles[k]->points(4);
        std::cout << "Obstacle: " << p[0] << p[1] << p[2] << p[3] << std::endl;
    }

    cv::imshow("image",undist);
    cv::waitKey(0);
}
