#include <eagle.h>

using namespace eagle;

int main(int argc, char* argv[]) {
    // read config file
    cv::FileStorage fs(CONFIG_PATH, cv::FileStorage::READ);
    std::string background_path = fs["detector"]["background_path"];
    fs.release();

    // robots and obstacles that the detector should search for
    Robot dave(0, 0.55, 0.4, cv::Scalar(138, 110, 17));
    Robot krist(1, 0.55, 0.4, cv::Scalar(17, 31, 138));
    Robot kurt(9, 0.55, 0.4, cv::Scalar(19, 138, 17));
    std::vector< Robot* > robots = std::vector< Robot* > {&dave, &krist, &kurt};
    std::vector< Obstacle* > obstacles;
    cv::Mat image, image2, background;

    background = cv::imread(background_path, CV_LOAD_IMAGE_COLOR);
    Detector detector(CONFIG_PATH, background);
    detector.verbose(2);

    // start camera
    Camera* cam = getCamera(CONFIG_PATH);
    cam->start();

    while ( !kbhit() ) {
        cam->read(image);
        detector.search(image, robots, obstacles);
        image2 = detector.draw(image, robots, obstacles);
        // show image
        imshow("Viewer", image2);
        cv::waitKey(1);
    }

    cam->stop();
    delete cam;
    return 0;
}
