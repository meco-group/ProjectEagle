#include "libcam.hpp"
#include "libcamsettings.hpp"
#include "utils.cpp"

using namespace cam;

int main(int argc, char* argv[]) {
    // Parse arguments
    const string cameraSettingsFile = argc > 1 ? argv[1] : "/home/peter/Documents/Honours/ProjectEagle/config/config.xml";

    // Open settings file
    CameraSettings cameraSettings;
    cameraSettings.read(cameraSettingsFile);

    // EXAMPLE_CAMERA_T cam(EXAMPLE_CAMERA_INDEX);
    V4L2Camera *cam = getCamera(cameraSettings.camIndex, cameraSettings.camType);

    // Start camera
    cam->setResolution(cameraSettings.res_width, cameraSettings.res_height);
    // cam->calibrate(cameraSettings.calPath); //camera can be calibrated
    cam->start();

    cv::Mat im;
    cv::namedWindow("Viewer",cv::WINDOW_AUTOSIZE);
    std::cout << "Hit enter to stop the program." << std::endl;

    while( !kbhit() ){
        cam->read(im);
        imshow("Viewer",im);
        cv::waitKey(1);
    }

    cam->stop();
    delete cam;
    return 0;
}
