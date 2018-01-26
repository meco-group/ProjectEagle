#include <eagle.h>

using namespace eagle;

int main(int argc, char* argv[]) {
    // parse arguments
    std::string output_path = (argc > 1) ? argv[1] : "../config/snapshot.png";
    bool undistort = (argc > 2) ? (strcmp(argv[2], "1") == 0) : true;

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

    // take snapshot
    cv::Mat im;
    for (int i=0; i<10; i++) {
        cam->read(im);
        cv::waitKey(1); // give camera time to reload
    }
    cam->stop();
    delete cam;

    // store snapshot
    cv::imwrite(output_path, im);
    return 0;
}
