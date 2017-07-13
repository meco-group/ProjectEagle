#include "cam/libcam.h"
#include "examples_config.h"
#include <opencv2/opencv.hpp>
#include "cam/camera_settings.h"

int main(int argc, char* argv[]) {
    // Parse arguments
    const string cameraSettingsFile = argc > 1 ? argv[1] : "../config/ceil1_cam.xml";
    const string outputFile = argc > 2 ? argv[2] : "../config/images_ceil1/snapshot.png";

    // Open settings file
    FileStorage fs("../config/ceil1_cam.xml", FileStorage::READ);
    CameraSettings cameraSettings;
    fs["CameraSettings"] >> cameraSettings;
    fs.release();

    // Instantiate camera
    V4L2Camera *cam = getCamera(cameraSettings.camIndex, cameraSettings.camType);
    cam->calibrate("../config/see3cam.yml"); //camera can be calibrated

    // Take snapshot
    Mat im;
    cam->start();
    for (int i=0; i<5; i++) {cam->read(im);}
    cam->stop();
    delete cam;

    // Store snapshot
    cv::imwrite(outputFile, im);
}

