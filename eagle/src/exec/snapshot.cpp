#include "libcam.hpp"
#include "libcamsettings.hpp"

using namespace eagle;

int main(int argc, char* argv[]) {
    // Parse arguments
    const string cameraSettingsFile = argc > 1 ? argv[1] : "../config/ceil1_cam.xml";
    const string outputFile = argc > 2 ? argv[2] : "../config/snapshot.png";

    // Open settings file
    CameraSettings cameraSettings;
    cameraSettings.read(cameraSettingsFile);

    // Instantiate camera
    V4L2Camera *cam = getCamera(cameraSettings.camIndex, cameraSettings.camType);
    cam->setResolution(cameraSettings.res_width, cameraSettings.res_height);
    // cam->calibrate(cameraSettings.calPath);

    // Take snapshot
    Mat im;
    cam->start();
    for (int i=0; i<15; i++) {
        cam->read(im);
        cv::waitKey(1); // give camera time to reload
    }
    cam->stop();
    delete cam;

    // Store snapshot
    cv::imwrite(outputFile, im);
}
