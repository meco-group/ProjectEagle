#include "libcam.hpp"
#include "libcom.hpp"
#include "libcamsettings.hpp"

using namespace com;
using namespace cam;

int main(int argc, char* argv[]) {
    // Parse arguments
    const string config = argc > 1 ? argv[1] : "/home/odroid/ProjectEagle/src/client/src/config/cal_confif.xml";

    CameraSettings cameraSettings;
    cameraSettings.read(config);
    ComSettings comSettings;
    comSettings.read(config);

    cout << "Starting ImageTransmitter - GROUP: "<<comSettings.group<<"\n";

    // EXAMPLE_CAMERA_T cam(EXAMPLE_CAMERA_INDEX);
    V4L2Camera *cam = getCamera(cameraSettings.camIndex, cameraSettings.camType);

    // Start camera
    cam->setResolution(cameraSettings.comp_res_width, cameraSettings.comp_res_height);
    cam->calibrate(cameraSettings.calPath); //camera can be calibrated
    cam->start();

    Communicator com("transmitter", comSettings.interface);
    com.start(comSettings.init_wait_time);
    com.join(comSettings.group);

    std::vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(90);
    std::vector<uchar> buffer(10000,0);

    // wait for peer
    std::cout << "waiting for peers" << std::endl;
    while(com.peers().size() <= 0) {
        sleep(1);
    }

    // print peers
    std::vector<std::string> peers = com.peers();
    std::cout << "peers: " << std::endl;
    for (auto &peer : peers) {
        std::cout << "* " << peer << std::endl;
    }

    // start capturing
    std::cout << "Start capturing video stream." << std::endl;

    uint img_id = 0;
    eagle::header_t header;
    header.id = eagle::IMAGE;

    Mat img;

    // Loop over all peers to send images_ceil1
    auto begin = std::chrono::system_clock::now();
    while(com.peers().size() > 0 ) {
        header.time = img_id;
        cam->read(img);

        Mat temp = img.clone();

        // Look for chessboard
        vector<Point2f> pointBuf;
        bool found = findChessboardCorners(img, Size(7, 6), pointBuf, 0); /*
                                       CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK |
                                       CV_CALIB_CB_NORMALIZE_IMAGE); */
        if (found) {
            Mat viewGray;
            cvtColor(img, viewGray, COLOR_BGR2GRAY);
            cornerSubPix(viewGray, pointBuf, Size(7, 6),
                         Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
            drawChessboardCorners(temp, Size(7, 6), Mat(pointBuf), found);
        }

    	cv::imencode(".jpg",temp, buffer, compression_params);
        if (com.shout(&header, buffer.data(), sizeof(header), buffer.size(), comSettings.group)) {
            std::cout << "Sending image " << img_id << ", size: " << buffer.size() << std::endl;
            std::cout << "Header size: "<<sizeof(header)<<"\n";
        }
        img_id++;
    }

    // Stop the loop: no peers connected
    auto end = std::chrono::system_clock::now();
    double fps = img_id/(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()*1e-6);
    std::cout << "Average framerate: " << fps << std::endl;

    // stop the program
    cam->stop();
    delete cam;
    com.leave(comSettings.group);
    com.stop();
}

