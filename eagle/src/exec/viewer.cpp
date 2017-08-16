#include "camera.h"
#include "utils.h"

using namespace eagle;

int main(int argc, char* argv[]) {
    Camera* cam = getCamera(CONFIG_PATH);
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
