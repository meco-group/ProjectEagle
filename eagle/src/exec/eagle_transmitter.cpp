#include "communicator.h"
#include "camera.h"
#include "detector.h"
#include "protocol.h"
#include "utils.h"
#include <string>
#include <chrono>

using namespace eagle;

int main(int argc, char* argv[]) {
    // parse arguments
    std::string node_name = (argc > 1) ? argv[1] : "eagle0";
    double frequency = (argc > 2) ? std::stod(argv[2]) : 10;
    bool image_stream = (argc > 3) ? (strcmp(argv[3], "1") == 0) : false;
    bool image_viewer = (argc > 4) ? (strcmp(argv[4], "1") == 0) : false;
    bool time = (argc > 5) ? (strcmp(argv[5], "1") == 0) : false;

    // read config file
    cv::FileStorage fs(CONFIG_PATH, cv::FileStorage::READ);
    std::string group = fs["communicator"]["group"];
    int zyre_wait_time = fs["communicator"]["zyre_wait_time"];
    int chb_w = fs["calibrator"]["board_width"];
    int chb_h = fs["calibrator"]["board_height"];
    cv::Mat camera_matrix, distortion_vector, external_tf, ground_plane;
    fs["camera"]["camera_matrix"] >> camera_matrix;
    fs["camera"]["distortion_vector"] >> distortion_vector;
    fs["camera"]["external_transformation"] >> external_tf;
    fs["camera"]["ground_plane"] >> ground_plane;
    fs.release();

    // setup video compression
    std::vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(30);
    std::vector<uchar> buffer(10000, 0);

    // start camera
    Camera* cam = getCamera(CONFIG_PATH);
    cam->undistort(camera_matrix, distortion_vector);
    cam->start();

    // start communicator
    Communicator com(node_name, CONFIG_PATH);
    com.start(zyre_wait_time);
    com.join(group);
    // setup detector
    cv::Matx33f cam2world_tf = image2ground_tf(ground_plane, camera_matrix, external_tf);
    Detector detector(CONFIG_PATH, cam2world_tf);

    // robots and obstacles that the detector should search for
    Robot dave(0, 0.55, 0.4, cv::Scalar(138, 110, 17));
    Robot krist(1, 0.55, 0.4, cv::Scalar(17, 31, 138));
    Robot kurt(9, 0.55, 0.4, cv::Scalar(19, 138, 17));
    std::vector< Robot* > robots = std::vector< Robot* >{&dave, &krist, &kurt};
    std::vector< Obstacle* > obstacles;

    std::cout << "Starting eagle transmitter\n";

    if (image_viewer) {
        cv::namedWindow("Viewer", cv::WINDOW_AUTOSIZE);
    }

    eagle::header_t imheader;
    imheader.id = eagle::IMAGE;
    cv::Mat im;
    // main execution loop
    int dt = int(1000./frequency);
    auto t0 = std::chrono::high_resolution_clock::now();
    auto begin = std::chrono::system_clock::now();
    auto end = std::chrono::system_clock::now();
    double t_cap, t_det, t_com;
    unsigned long capture_time;
    while ( !kbhit() ) {
        //  check time
        auto t = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(t-t0).count() < dt) {
            continue;
        }
        t0 = t;
        if (time) {begin = std::chrono::system_clock::now();}
        // capture image
        cam->read(im);
        capture_time = timestamp();
        if (time) {
            end = std::chrono::system_clock::now();
            t_cap = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()*1e-6;
            begin = std::chrono::system_clock::now();
        }
        // detect the robots/obstacles
        detector.search(im, robots, obstacles);
        if (time) {
            end = std::chrono::system_clock::now();
            t_det = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()*1e-6;
            begin = std::chrono::system_clock::now();
        }
        // pack robots
        size_t n_robots = 0;
        eagle::header_t mheaders[robots.size()];
        eagle::marker_t markers[robots.size()];
        for (int k = 0; k < robots.size(); k++) {
            if( robots[k]->detected() ) {
                mheaders[n_robots].id = eagle::MARKER;
                mheaders[n_robots].time = capture_time;
                markers[n_robots] = robots[k]->serialize();
                n_robots++;
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
        std::vector< size_t > sizes = std::vector< size_t >(2*n_robots + 2*obstacles.size(),0);
        std::vector< const void* > data = std::vector< const void* >(sizes.size());
        int i = 0;
        for (int k = 0; k < n_robots; k++) {
            sizes[i] = sizeof(eagle::header_t);
            data[i++] = (const void*)(&mheaders[k]);
            sizes[i] = sizeof(eagle::marker_t);
            data[i++] = (const void*)(&markers[k]);
        }
        for (int k = 0; k < obstacles.size(); k++) {
            sizes[i] = sizeof(eagle::header_t);
            data[i++] = (const void*)(&oheaders[k]);
            sizes[i] = sizeof(eagle::obstacle_t);
            data[i++] = (const void*)(&obs[k]);
        }
        // send everything
        com.shout(data, sizes, group);
        if (time) {
            end = std::chrono::system_clock::now();
            t_com = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()*1e-6;
        }
        if (image_viewer || image_stream) {
            detector.draw(im, robots, obstacles);
        }
        if (image_stream) {
            // draw everything on img
            imheader.time = capture_time;
            cv::imencode(".jpg", im, buffer, compression_params);
            com.shout(&imheader, buffer.data(), sizeof(imheader), buffer.size(), group);
        }
        if (image_viewer) {
            cv::imshow("Viewer", im);
            cv::waitKey(1);
        }
        // print out
        std::cout << "Detected " << n_robots << " robot(s) ";
        if (n_robots > 0) {
            int rob_cnt = 0;
            std::cout << "[";
            for (uint k=0; k<robots.size(); k++) {
                if (robots[k]->detected()) {
                    std::cout << robots[k]->code();
                    rob_cnt++;
                    if (rob_cnt < n_robots) {
                        std::cout << ",";
                    }
                }
            }
            std:: cout << "]";
        }
        std::cout << " and " << obstacles.size() << " obstacle(s)." << std::endl;
        if (time) {
            std::cout << "t_cap: " << t_cap << "s - t_det: " << t_det << "s - t_com: " << t_com << "s" << std::endl;
        }
    }
    // stop the program
    cam->stop();
    delete cam;
    com.leave(group);
    com.stop();
    return 0;
}
