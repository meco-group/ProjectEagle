#include "cam/libcam.h"
#include "examples_config.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "cam/camera_settings.h"

using namespace std;
using namespace cv;

int main(int argc, char* argv[]) {
    // Parse arguments
    const string cameraSettingsFile = argc > 1 ? argv[1] : "../config/ceil1_cam.xml";

    // Open settings file
    FileStorage fs(cameraSettingsFile, FileStorage::READ);
    CameraSettings cameraSettings;
    fs["CameraSettings"] >> cameraSettings;
    fs.release();

	// EXAMPLE_CAMERA_T cam(EXAMPLE_CAMERA_INDEX);
    V4L2Camera *cam = getCamera(cameraSettings.camIndex, cameraSettings.camType);

    // Start camera
    cam->calibrate(cameraSettings.calPath); //camera can be calibrated
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
