#include <comm/communicator.h>
#include <thread>
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
    // cam->setBrightness(5);
    // cam->setExposure(100000);

    if (!transmit) {
        cv::namedWindow("Viewer", cv::WINDOW_AUTOSIZE);
        std::cout << "Hit g to take a snapshot" << std::endl;
    }

    struct X {
        bool stop;
        bool transmit;
        V4L2Camera* cam;
        Communicator* com;
        eagle::header_t header;
        std::vector<int> compression_params;
        std::vector<uchar> buffer;
        cv::Mat im;
        int img_id = 0;

        X(CameraSettings cameraSettings) : stop(false), transmit(transmit) {

            // Open communicator
            com = new Communicator("eagle", EXAMPLE_COMMUNICATOR_INTERFACE);

            // setup video compression
            compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
            compression_params.push_back(30);
            buffer = std::vector<uchar>(10000, 0);

            if (transmit) {
                com->start();
                com->join("EAGLE");

                // wait for peer
                std::cout << "waiting for peers" << std::endl;
                while (com->peers().size() <= 0) {
                    sleep(1);
                }
            }

            // Create the header
            img_id = 0;
            header.id = eagle::IMAGE;

            // EXAMPLE_CAMERA_T cam(EXAMPLE_CAMERA_INDEX);
            cam = getCamera(cameraSettings.camIndex, cameraSettings.camType);

            // Start camera
            // cam->calibrate("../config/see3cam.yml"); //camera can be calibrated
            cam->start();

        };
        void loop() {
            while (!stop) {
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
                    if (com->peers().size() <= 0)
                        break;

                    header.time = img_id;
                    cv::imencode(".jpg", im, buffer, compression_params);
                    com->shout(&header, buffer.data(), sizeof(header), buffer.size(), "EAGLE");
                    img_id++;
                } else {
                    imshow("Viewer", temp);
                }
            }

            stop = false;

        }
    };

    X x = {cameraSettings};

    for (int i= 0; i<calSettings.imageCount; i++) {
        const string outputFile = calSettings.imageList[i];

        x.stop = false;
        std::thread t(&X::loop, &x);
        t.join();

        cin.get();

        x.stop = true;
        while (x.stop) {}

        // Store snapshot
        cv::imwrite(outputFile, x.im);
        cout << "took snapshot: " << outputFile << "\n";
    }

    x.cam->stop();
    delete x.cam;

    // stop the program
    x.com->leave("EAGLE");
    x.com->stop();
    delete x.com;
}
