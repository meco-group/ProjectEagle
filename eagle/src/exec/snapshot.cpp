#include "camera.h"
#include "utils.h"

using namespace eagle;

int main(int argc, char* argv[]) {
    // Parse arguments
    const string cameraSettingsFile = argc > 1 ? argv[1] : "../config/ceil1_cam.xml";
    const string outputFile = argc > 2 ? argv[2] : "../config/snapshot.png";

    // start camera
    Camera* cam = getCamera(CONFIG_PATH);
    cam->start();

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
