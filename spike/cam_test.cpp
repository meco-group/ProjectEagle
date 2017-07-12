#include <comm/communicator.h>
#include <thread>
#include <atomic>
#include "experiments.h"

void detect_pattern(string config, bool transmit) {
    cout << "Taking Calibration snaps for config: "
         << config << "\n";

    // Open settings file
    FileStorage fs(config, FileStorage::READ);
    CameraSettings cameraSettings;
    fs["CameraSettings"] >> cameraSettings;
    CalSettings calSettings;
    fs["CalibrationSettings"] >> calSettings;
    fs.release();

    cout << "Opening camera: "
         << cameraSettings.camIndex << " of type: "
         << getCamType(cameraSettings.camType) << "\n";

    // Open communicator
    Communicator com("eagle", EXAMPLE_COMMUNICATOR_INTERFACE);

    // setup video compression
    std::vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(5);
    std::vector<uchar> buffer(20, 0);

    if (transmit) {
        com.start();
        com.join("EAGLE");

        // wait for peer
        std::cout << "waiting for peers" << std::endl;
        while (com.peers().size() <= 0) {
            sleep(1);
        }
    }

    // Create the header
    int img_id = 0;
    eagle::header_t header;
    header.id = eagle::IMAGE;

    // EXAMPLE_CAMERA_T cam(EXAMPLE_CAMERA_INDEX);
    V4L2Camera *cam = getCamera(cameraSettings.camIndex, cameraSettings.camType);
    // cam->setBrightness(5);
    // cam->setExposure(100000);

    // Start camera
    // cam->calibrate("../config/see3cam.yml"); //camera can be calibrated
    cam->start();

    cv::Mat im;
    if (!transmit) {
        cv::namedWindow("Viewer", cv::WINDOW_AUTOSIZE);
    }

    for (int i= 0; i<calSettings.imageCount; i++) {
        const string outputFile = calSettings.imageList[i];

        bool stopper = false;
        thread stopThread(userStop, &stopper);      // start thread looking for user input

        while (true) {
            if(stopper) { stopThread.join(); break; }

            cam->read(im);

            Mat temp = im.clone();

            // Look for chessboard
            vector<Point2f> pointBuf;
            bool found = findChessboardCorners(im, Size(7, 6), pointBuf, 0); /*
                                       CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK |
                                       CV_CALIB_CB_NORMALIZE_IMAGE); */

            if (found) {
                Mat viewGray;
                cvtColor(im, viewGray, COLOR_BGR2GRAY);
                cornerSubPix(viewGray, pointBuf, Size(7, 6),
                             Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
                drawChessboardCorners(temp, Size(7, 6), Mat(pointBuf), found);
            }

            // Transmit image
            if (transmit) {
                if (com.peers().size() <= 0)
                    break;

                header.time = img_id;
                resize(temp, temp, Size(), .4, .4, cv::INTER_LANCZOS4);
                cv::imencode(".jpg", temp, buffer, compression_params);
                com.shout(&header, buffer.data(), sizeof(header), buffer.size(), "EAGLE");
                cout << "buffer size: "<<buffer.size()<<"\n";
		img_id++;

            } else {
                imshow("Viewer", temp);
                cv::waitKey(1);
            }
        }

        // Store snapshot
        cv::imwrite(outputFile, im);
        cout << "took snapshot: " << outputFile << "\n";
    }

    cam->stop();
    delete cam;

    // stop the program
    com.leave("EAGLE");
    com.stop();
}

void userStop(bool *st) {
    char chChar;
    while (chChar != '\n')
        chChar = getchar();
    *st = true;
}
