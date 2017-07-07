#include "experiments.h"

void cam_test() {
    CameraSettings sc;
    sc.calPath = "../config/cei"
            "l2_cam_cal.xml";
    sc.camType = PICAM;
    sc.camIndex = 0;

    // Generate the settings file
    BoardSettings sb;
    sb.boardSize = Size(7, 6);
    sb.squareSize = 108;
    sb.calibrationPattern = sb.CHESSBOARD;

    // Open and write file
    FileStorage fs("../config/ceil2_cam.xml", FileStorage::WRITE);
    sc.write(fs);
    sb.write(fs);
    fs.release();
}

void detect_pattern() {

    // Parse arguments
    const string cameraSettingsFile = "../config/ceil2_cam.xml";

    // Open settings file
    FileStorage fs(cameraSettingsFile, FileStorage::READ);
    CameraSettings cameraSettings;
    fs["CameraSettings"] >> cameraSettings;
    fs.release();

    // EXAMPLE_CAMERA_T cam(EXAMPLE_CAMERA_INDEX);
    See3Camera* cam = (See3Camera*)getCamera(cameraSettings.camIndex, cameraSettings.camType);
    cam->setBrightness(5);
    cam->setExposure(100000);

    // Start camera
    // cam->calibrate("../config/see3cam.yml"); //camera can be calibrated
    cam->start();

    cv::Mat im;
    cv::namedWindow("Viewer",cv::WINDOW_AUTOSIZE);
    std::cout << "Hit g to take a snapshot" << std::endl;

    for (int i= 0; i<10; i++) {
        const string outputFile = "../config/images/"+std::to_string(i)+".jpg";

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
