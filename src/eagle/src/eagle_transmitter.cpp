#include "libcam.hpp"
#include "libcamsettings.hpp"
#include "libcom.hpp"
#include "helpers/detector.h"
#include "libio.hpp"
#include <string>
#include <chrono>

using namespace com;
using namespace cam;

int main(int argc, char* argv[]) {
    // Parse arguments
    string config = argc > 1 ? argv[1] : "/home/odroid/ProjectEagle/src/client/config/devices/eagle0/config.xml";
    string node_name = argc > 2 ? argv[2] : "eagle0";
    string frequency_str = argc > 3 ? argv[3] : "10"; // Hz
    string image_stream_str = argc > 4 ? argv[4] : "1";
    double frequency = std::stod(frequency_str);
    bool image_stream = (image_stream_str == "1");

    // open settings file
    CameraSettings cameraSettings;
    cameraSettings.read(config);
    ComSettings comSettings;
    comSettings.read(config);

    V4L2Camera *cam = getCamera(cameraSettings.camIndex, cameraSettings.camType);

    // start camera
    cam->setResolution(cameraSettings.res_width, cameraSettings.res_height);
    cam->calibrate(cameraSettings.calPath);
    cam->start();

    // setup the communication
    Communicator com(node_name, comSettings.interface);
    com.start(comSettings.init_wait_time);
    com.join(comSettings.group);

    // setup video compression
    std::vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(30);
    std::vector<uchar> buffer(10000,0);

    eagle::header_t imheader;
    imheader.id = eagle::IMAGE;

    // create detector
    Detector detector(cameraSettings.detPath, cameraSettings.bgPath);

    Mat im;
    Mat R; Mat T;
    cv::FileStorage fs(cameraSettings.extPath, cv::FileStorage::READ);
    fs["rotation_matrix"] >> R;
    fs["translation_matrix"] >> T;
    fs.release();

    // make robots which the camera should find
    Robot dave(0, 0.55, 0.4, cv::Scalar(17, 110, 138), T, R);
    Robot krist(1, 0.55, 0.4, cv::Scalar(138, 31, 17), T, R);
    Robot kurt(9, 0.55, 0.4, cv::Scalar(17, 138, 19), T, R);

    std::vector< Robot* > robots = std::vector< Robot* >{&dave, &krist, &kurt};
    std::vector< Obstacle* > obstacles;

    std::cout << "Starting eagle transmitter\n";
    std::cout << "T: \n" << T << "\n";

    // main execution loop
    int dt = 1000/frequency;
    auto t0 = std::chrono::high_resolution_clock::now();
    double capture_time;
    double capture_time2;
    while ( !io::kbhit() ) {
        //  check time
        auto t = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(t-t0).count() < dt) {
            continue;
        }
        t0 = t;
        // capturer image
        cam->read(im);
        capture_time = cam->captureTime();
        capture_time2 = cam->getV4L2CaptureTime();
        std::cout << "[" << capture_time << " vs " << capture_time2 << "]" << std::endl;
        // detect the robots/obstacles
        detector.search(im, robots, obstacles);

        // pack robots
        size_t n_detected_robots = 0;
        eagle::header_t mheaders[robots.size()];
        eagle::marker_t markers[robots.size()];
        for (int k = 0; k < robots.size(); k++) {
            if( robots[k]->detected() ) {
                mheaders[n_detected_robots].id = eagle::MARKER;
                mheaders[n_detected_robots].time = capture_time;
                markers[n_detected_robots++] = robots[k]->serialize();
            }
        }
        // pack obstacles
        eagle::header_t oheaders[obstacles.size()];
        eagle::obstacle_t obs[obstacles.size()];
        for (int k = 0; k < obstacles.size(); k++) {
            oheaders[k].id = eagle::OBSTACLE;
            oheaders[k].time = capture_time;
            obs[k] = obstacles[k]->serialize();
        }

        // make data and size vector
        std::vector< size_t > sizes = std::vector< size_t >(2*n_detected_robots + 2*obstacles.size(),0);
        std::vector< const void* > data = std::vector< const void* >(sizes.size());

        int i = 0;
        for (int k = 0; k < n_detected_robots; k++) {
            sizes[i++] = sizeof(eagle::header_t);
            data[i++] = (const void*)(&mheaders[k]);
            sizes[i++] = sizeof(eagle::marker_t);
            data[i++] = (const void*)(&markers[k]);
        }

        for (int k = 0; k < obstacles.size(); k++) {
            sizes[i++] = sizeof(eagle::header_t);
            data[i++] = (const void*)(&oheaders[k]);
            sizes[i++] = sizeof(eagle::obstacle_t);
            data[i++] = (const void*)(&obs[k]);
        }

        // send everything
        com.shout(data, sizes, comSettings.group);

        if (image_stream) {
            // draw everything to give some feedback
            detector.draw(im, robots, obstacles);
            imheader.time = capture_time;
            cv::imencode(".jpg", im, buffer, compression_params);
            com.shout(&imheader, buffer.data(), sizeof(imheader), buffer.size(), comSettings.group);
        }

        for (int k=0; k<robots.size(); k++) {
            if (robots[k]->detected()) {
                std::cout << "Robot " << robots[k]->code() << " detected!" << std::endl;
            }
        }
    }

    // stop the program
    cam->stop();
    delete cam;
    com.leave(comSettings.group);
    com.stop();
}
