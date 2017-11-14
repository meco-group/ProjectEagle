#include <eagle.h>

using namespace eagle;

int main(int argc, char* argv[]) {
    // parse arguments
    bool undistort = (argc > 1) ? (strcmp(argv[1], "1") == 0) : true;
    double frequency = (argc > 2) ? std::stod(argv[2]) : 1000;

    // read config file
    cv::FileStorage fs(CONFIG_PATH, cv::FileStorage::READ);
    cv::Mat camera_matrix, distortion_vector;
    fs["camera"]["camera_matrix"] >> camera_matrix;
    fs["camera"]["distortion_vector"] >> distortion_vector;
    fs.release();

    // start camera
    Camera* cam = getCamera(CONFIG_PATH);
    if (undistort) {
        cam->undistort(camera_matrix, distortion_vector);
    }
    cam->start();

    cv::Mat im;
    cv::namedWindow("Viewer", cv::WINDOW_AUTOSIZE);
    std::cout << "Hit enter to stop the program." << std::endl;

    // main execution loop
    int dt = int(1000. / frequency);
    auto t0 = std::chrono::high_resolution_clock::now();
    while ( !kbhit() ) {
        // check time
        auto t = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(t - t0).count() < dt) {
            continue;
        }
        t0 = t;
        // show image
        cam->read(im);
        imshow("Viewer", im);
        cv::waitKey(1);
    }

    cam->stop();
    delete cam;
    return 0;
}
