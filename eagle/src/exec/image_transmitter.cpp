#include "communicator.h"
#include "camera.h"
#include "protocol.h"
#include "utils.h"

using namespace eagle;

int main(int argc, char* argv[]) {
    // parse arguments
    std::string node_name = (argc > 1) ? argv[1] : "eagle0";
    bool undistort = (argc > 2) ? (strcmp(argv[2], "1") == 0) : true;
    bool detect_chb = (argc > 3) ? (strcmp(argv[3], "1") == 0) : true;
    bool snap_cmd = (argc > 4) ? (strcmp(argv[4], "1") == 0) : false;
    std::string snapshot_path = (argc > 5) ? argv[5] : "../config/snapshot.png";

    // read config file
    cv::FileStorage fs(CONFIG_PATH, cv::FileStorage::READ);
    std::string group = fs["communicator"]["group"];
    int zyre_wait_time = fs["communicator"]["zyre_wait_time"];
    int chb_w = fs["calibrator"]["board_width"];
    int chb_h = fs["calibrator"]["board_height"];
    cv::Mat camera_matrix, distortion_vector;
    fs["camera"]["camera_matrix"] >> camera_matrix;
    fs["camera"]["distortion_vector"] >> distortion_vector;
    fs.release();

    // setup video compression
    std::vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(90);
    std::vector<uchar> buffer(10000, 0);

    // start camera
    Camera* cam = getCamera(CONFIG_PATH);
    if (undistort) {
        cam->undistort(camera_matrix, distortion_vector);
    }
    cam->start();

    // start communicator
    Communicator com(node_name, CONFIG_PATH);
    com.join(group);
    com.start(zyre_wait_time);

    // wait for peers
    std::cout << "Waiting for peers ... ";
    while(com.peers().size() <= 0) {
        sleep(1);
    }
    std::cout << "done." << std::endl;

    // start capturing
    std::cout << "Start transmitting video stream." << std::endl;

    eagle::header_t header;
    header.id = eagle::IMAGE;

    cv::Mat img;
    uint img_cnt = 0;
    auto begin = std::chrono::system_clock::now();
    while(com.peers().size() > 0 ) {
        cam->read(img);
        header.time = timestamp();
        
        if (snap_cmd) {
            std::string pr;
            eagle::header_t header;
            eagle::cmd_t cmd;
            if (com.receive(pr)) {
                while (com.available()) {
                    // 1. read the header
                    com.read(&header);
                    size_t size = com.framesize();
                    uchar buffer2[size];
                    com.read(buffer2);
                    // 2. read data based on the header
                    if (header.id == eagle::CMD) {
                        cmd = *((eagle::cmd_t*)(buffer2));
                        if (cmd == SNAPSHOT) {
                            cv::imwrite(snapshot_path, img);
                            std::cout << "Took snapshot." << std::endl;
                        }
                        break;
                    }
                }
            }
        }
        
        if (detect_chb) {
            // look for chessboard
            std::vector<cv::Point2f> pnt_buf;
            if (cv::findChessboardCorners(img, cv::Size(chb_h, chb_w), pnt_buf, 0)) {
                cv::Mat img_gray;
                cv::cvtColor(img, img_gray, cv::COLOR_BGR2GRAY);
                cv::cornerSubPix(img_gray, pnt_buf, cv::Size(chb_h, chb_w), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
                cv::drawChessboardCorners(img, cv::Size(chb_h, chb_w), cv::Mat(pnt_buf), true);
            }
        }
        // compress image
    	  cv::imencode(".jpg", img, buffer, compression_params);
        // transmit image
        if (com.shout(&header, buffer.data(), sizeof(header), buffer.size(), group)) {
            // std::cout << "Sending image " << img_cnt << ", size: " << buffer.size() << " - ";
            // std::cout << "Header size: "<< sizeof(header) << "\n";
        }
        img_cnt++;
    }

    // Stop the loop: no peers connected
    auto end = std::chrono::system_clock::now();
    double fps = img_cnt/(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()*1e-6);
    std::cout << "Stopped image transmission (average framerate: " << fps << ")." << std::endl;

    // stop the program
    cam->stop();
    delete cam;
    com.leave(group);
    com.stop();
    return 0;
}

