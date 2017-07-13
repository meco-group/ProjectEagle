#include <cal/calibrator.h>

using namespace cv;
using namespace std;

int main(int argc, char* argv[]) {
    CalSettings s;
    const string inputSettingsFile = argc > 1 ? argv[1] : "default.xml";
    Mat cameraMatrix, distCoeffs;
    try {
        Calibrator::calibrate(inputSettingsFile, cameraMatrix, distCoeffs);
    } catch (invalid_argument) {
        cout << "oh noes\n";
    }
}