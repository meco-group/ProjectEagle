#include "libcam.hpp"
#include "libcamsettings.hpp"

using namespace cam;

int main(int argc, char* argv[]) {
    // Parse arguments
    const string cameraSettingsFile = argc > 1 ? argv[1] : "/home/odroid/ProjectEagle/src/client/config/devices/eagle0/config.xml";

    // Open settings file
    CameraSettings cameraSettings;
    cameraSettings.read(cameraSettingsFile);

    // EXAMPLE_CAMERA_T cam(EXAMPLE_CAMERA_INDEX);
    V4L2Camera *cam = getCamera(cameraSettings.camIndex, cameraSettings.camType);

    // Start camera
    cam->setResolution(cameraSettings.res_width, cameraSettings.res_height);
    cam->calibrate(cameraSettings.calPath); //camera can be calibrated
    cam->start();

    // capture and save background
    cv::Mat frame, background;
    cam->read(frame);
    background = cv::Mat(frame.size(), CV_32FC3, cv::Scalar(0,0,0));
    for (int i=0; i<50; i++) {
        cam->read(frame);
        cv::accumulate(frame, background);
    }
    background /= 50;
    background.convertTo(background, CV_8UC3);
    cv::imwrite(cameraSettings.bgPath, background);
    imshow("Background", background);
    cv::waitKey(2000);

    cam->stop();
    delete cam;
    return 0;
}

