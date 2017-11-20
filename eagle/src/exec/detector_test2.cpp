#include <eagle.h>

using namespace eagle;

int main(int argc, char* argv[]) {
    double frequency = (argc > 1) ? std::stod(argv[1]) : 1000;

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

    int dt = int(1000. / frequency);
    auto t0 = std::chrono::high_resolution_clock::now();
    while ( !kbhit() ) {
        // check time
        auto t = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(t - t0).count() < dt) {
            continue;
        }
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
