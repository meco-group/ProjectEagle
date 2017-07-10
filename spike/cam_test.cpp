#include "experiments.h"

void detect_pattern(string config) {
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

    // EXAMPLE_CAMERA_T cam(EXAMPLE_CAMERA_INDEX);
    V4L2Camera* cam = getCamera(cameraSettings.camIndex, cameraSettings.camType);
    // cam->setBrightness(5);
    // cam->setExposure(100000);

    // Start camera
    // cam->calibrate("../config/see3cam.yml"); //camera can be calibrated
    cam->start();

    cv::Mat im;
    cv::namedWindow("Viewer",cv::WINDOW_AUTOSIZE);
    std::cout << "Hit g to take a snapshot" << std::endl;

    for (int i= 0; i<calSettings.imageCount; i++) {
        const string outputFile = calSettings.imageList[i];

        while (true) {
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

            imshow("Viewer", temp);
            char key = (char)waitKey(1);

            if (key == 'g') {
                break;
            }
        }

        // Store snapshot
        cv::imwrite(outputFile, im);
        cout << "took snapshot: " << outputFile << "\n";
    }

    cam->stop();
    delete cam;
}
