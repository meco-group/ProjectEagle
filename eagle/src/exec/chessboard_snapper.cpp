#include <eagle.h>
#include <chrono>

using namespace eagle;

int main(int argc, char* argv[]) {
    // parse arguments
    std::string output_folder = (argc > 1) ? argv[1] : "../config/calibration/";
    double update_frequency = (argc > 2) ? std::stod(argv[2]) : 0.2;

    // read config file
    cv::FileStorage fs(CONFIG_PATH, cv::FileStorage::READ);
    int chb_w = fs["calibrator"]["board_width"];
    int chb_h = fs["calibrator"]["board_height"];
    fs.release();

    // start camera
    Camera* cam = getCamera(CONFIG_PATH);
    cam->start();

    // give camera time to reload
    cv::Mat im;
    for (int i = 0; i < 10; i++) {
        cam->read(im);
        cv::waitKey(1);
    }

    // take snapshots
    int cnt = 0;
    int dt = int(1000./update_frequency);
    auto t0 = std::chrono::high_resolution_clock::now();
    std::vector<cv::Point2f> pnt_buf;
    while ( !kbhit() ) {
        // read camera
        cam->read(im);
        // look for chessboard
        auto t = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(t-t0).count() > dt) {
            t0 = t;
            if (cv::findChessboardCorners(im, cv::Size(chb_h, chb_w), pnt_buf, 0)) {
                std::string output_path = output_folder;
                output_path.append("snapshot");
                output_path.append(std::to_string(cnt));
                output_path.append(".png");
                cv::imwrite(output_path, im);
                std::cout << "snapshot saved as " << output_path << std::endl;
                cnt++;
                // draw chessboard
                cv::drawChessboardCorners(im, cv::Size(chb_h, chb_w), cv::Mat(pnt_buf), true);
                cv::imshow("Viewer", im);
                cv::waitKey(500);
            }
        } else {
            cv::imshow("Viewer", im);
            cv::waitKey(1);
        }
    }
    cam->stop();
    delete cam;

}
