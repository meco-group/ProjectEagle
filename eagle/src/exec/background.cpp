#include <eagle.h>

using namespace eagle;

int main(int argc, char* argv[]) {
    // read config file
    cv::FileStorage fs(CONFIG_PATH, cv::FileStorage::READ);
    std::string background_path = fs["detector"]["background_path"];
    cv::Mat camera_matrix, distortion_vector;
    fs["camera"]["camera_matrix"] >> camera_matrix;
    fs["camera"]["distortion_vector"] >> distortion_vector;
    fs.release();

    // start camera
    Camera* cam = getCamera(CONFIG_PATH);
    cam->undistort(camera_matrix, distortion_vector);
    cam->start();

    // capture and save background
    cv::Mat frame, background;
    cam->read(frame);
    background = cv::Mat(frame.size(), CV_32FC3, cv::Scalar(0, 0, 0));
    for (int i=0; i<10; i++) {
        cam->read(frame);
        cv::waitKey(1); // give camera time to reload
    }
    for (int i=0; i<50; i++) {
        cam->read(frame);
        cv::accumulate(frame, background);
    }
    background /= 50;
    background.convertTo(background, CV_8UC3);
    cv::imwrite(background_path, background);
    imshow("Background", background);
    cv::waitKey(2000);

    cam->stop();
    delete cam;
    return 0;
}

