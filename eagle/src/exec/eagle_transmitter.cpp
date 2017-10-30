#include "communicator.h"
#include "camera.h"
#include "detector.h"
#include "protocol.h"
#include "utils.h"
#include <string>
#include <chrono>

using namespace eagle;

typedef struct settings_t {
    bool image_stream_on;
    bool image_viewer_on;
    bool detection_on;
    bool calibration_on;
    bool snapshot;
    bool debug_mode_on;
};

void transmit_detected(Communicator& com, const std::vector<Robot*>& robots, const std::vector<Obstacle*>& obstacles, std::string group, unsigned long capture_time) {
    // pack robots
    size_t n_robots = 0;
    eagle::header_t mheaders[robots.size()];
    eagle::marker_t markers[robots.size()];
    for (int k = 0; k < robots.size(); k++) {
        if( robots[k]->detected() ) {
            mheaders[n_robots].id = eagle::MARKER;
            mheaders[n_robots].time = capture_time;
            markers[n_robots] = robots[k]->serialize();
            n_robots++;
        }
    }

    // pack obstacles
    eagle::header_t oheaders[obstacles.size()];
    eagle::obstacle_t obs[obstacles.size()];
    for (int k = 0; k < obstacles.size(); k++) {
        oheaders[k].id = eagle::OBSTACLE;
        oheaders[k].time = capture_time;
        obs[k] = obstacles[k]->serialize();
    }

    // make data and size vector
    std::vector< size_t > sizes = std::vector< size_t >(2*n_robots + 2*obstacles.size(),0);
    std::vector< const void* > data = std::vector< const void* >(sizes.size());
    int i = 0;
    for (int k = 0; k < n_robots; k++) {
        sizes[i] = sizeof(eagle::header_t);
        data[i++] = (const void*)(&mheaders[k]);
        sizes[i] = sizeof(eagle::marker_t);
        data[i++] = (const void*)(&markers[k]);
    }
    for (int k = 0; k < obstacles.size(); k++) {
        sizes[i] = sizeof(eagle::header_t);
        data[i++] = (const void*)(&oheaders[k]);
        sizes[i] = sizeof(eagle::obstacle_t);
        data[i++] = (const void*)(&obs[k]);
    }

    // send everything
    com.shout(data, sizes, group);
}

void process_communication(Communicator& com, settings_t& settings) {
    std::string pr;
    eagle::header_t header;
    if (com.receive(pr)) {
        while (com.available()) {
            // 1. read the header
            com.read(&header);
            size_t size = com.framesize();
            uchar buffer[size];
            com.read(buffer);
            // 2. read data based on the header
            if (header.id == eagle::CMD) {
                eagle::cmd_t cmd;
                cmd = *((eagle::cmd_t*)(buffer));
                switch(cmd) {
                    case SNAPSHOT: {
                        settings.snapshot = true;
                        break; }
                    case IMAGE_STREAM_ON: {
                        settings.image_stream_on = true;
                        break; }
                    case IMAGE_STREAM_OFF: {
                        settings.image_stream_on = false;
                        break; }
                    case IMAGE_STREAM_TOGGLE: {
                        settings.image_stream_on = !settings.image_stream_on;
                        break; }
                    case DETECTION_ON: {
                        settings.detection_on = true;
                        break; }
                    case DETECTION_OFF: {
                        settings.detection_on = false;
                        break; }
                    case DETECTION_TOGGLE: {
                        settings.detection_on = !settings.detection_on;
                        break; }
                    case DEBUG_MODE_ON: {
                        settings.debug_mode_on = true;
                        break; }
                    case DEBUG_MODE_OFF: {
                        settings.debug_mode_on = false;
                        break; }
                    case DEBUG_MODE_TOGGLE: {
                        settings.debug_mode_on = !settings.debug_mode_on;
                        break; }
                    case CALIBRATION_ON: {
                        settings.calibration_on = true;
                        break; }
                    case CALIBRATION_OFF: {
                        settings.calibration_on = false;
                        break; }
                    case CALIBRATION_TOGGLE: {
                        settings.calibration_on = !settings.calibration_on;
                        break; }
                    default: { std::cout << "unknown command." << std::endl; }
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    // default settings
    settings_t settings;

    // parse arguments
    std::string node_name = (argc > 1) ? argv[1] : "eagle0";
    double update_frequency = (argc > 2) ? std::stod(argv[2]) : 10;
    settings.image_stream_on = (argc > 3) ? (strcmp(argv[3], "1") == 0) : false;
    settings.image_viewer_on = (argc > 4) ? (strcmp(argv[4], "1") == 0) : false;
    bool time = (argc > 5) ? (strcmp(argv[5], "1") == 0) : false;
    settings.detection_on = true;
    settings.calibration_on = false;
    settings.debug_mode_on = true;
    settings.snapshot = false;

    /******************/
    /* INITIALIZATION */
    /******************/

    // read config file
    cv::FileStorage fs(CONFIG_PATH, cv::FileStorage::READ);
    std::string group = fs["communicator"]["group"];
    int zyre_wait_time = fs["communicator"]["zyre_wait_time"];
    fs.release();

    // setup video compression
    std::vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(30);
    std::vector<uchar> buffer(10000, 0);

    // start camera
    Camera* cam = getCamera(CONFIG_PATH);
    cam->start();

    // start communicator
    Communicator com(node_name, CONFIG_PATH);
    com.start(zyre_wait_time);
    com.join(group);
    
    // setup detector
    Detector detector(CONFIG_PATH);

    // setup pattern extractor
    PatternExtractor extractor(CONFIG_PATH);

    // robots and obstacles that the detector should search for
    Robot dave(0, 0.55, 0.4, cv::Scalar(138, 110, 17));
    Robot krist(1, 0.55, 0.4, cv::Scalar(17, 31, 138));
    Robot kurt(9, 0.55, 0.4, cv::Scalar(19, 138, 17));
    std::vector< Robot* > robots = std::vector< Robot* >{&dave, &krist, &kurt};
    std::vector< Obstacle* > obstacles;

    std::cout << "Starting eagle transmitter\n";

    if (settings.image_viewer_on) {
        cv::namedWindow("Viewer", cv::WINDOW_AUTOSIZE);
    }

    /*************/
    /* MAIN LOOP */
    /*************/

    INIT_TIMING;

    eagle::header_t imheader;
    imheader.id = eagle::IMAGE;
    cv::Mat im;

    int dt = int(1000./update_frequency); //in milliseconds
    auto t0 = std::chrono::high_resolution_clock::now(); 
    double t_cap, t_det, t_com;
    unsigned long capture_time;
    while ( !kbhit() ) {
        //  check time
        auto t = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(t-t0).count() < dt) {
            process_communication(com, settings);
            continue;
        }
        t0 = t;
        
        // capture image
        TIMING(cam->read(im); capture_time = timestamp(););
        t_cap = DURATION;

        if (settings.snapshot) {
            settings.snapshot = false;
            cv::imwrite("../config/snapshot.png", im);
            std::cout << "Snapshot taken." << std::endl;
        }

        if (settings.detection_on) {
            // detect the robots/obstacles
            TIMING(detector.search(im, robots, obstacles););
            t_det = DURATION;
            // send detected robots/obstacles
            TIMING(transmit_detected(com, robots, obstacles, group, capture_time););
            t_com = DURATION;
            if ((settings.image_viewer_on || settings.image_stream_on) && settings.debug_mode_on) {
                detector.draw(im, robots, obstacles);
            }
        }

        if (settings.calibration_on) {
            int t;
            extractor.extract(im, cv::Point2f(0,0), t, true);
        }

        if (settings.image_stream_on) {
            // draw everything on img
            imheader.time = capture_time;
            cv::imencode(".jpg", im, buffer, compression_params);
            com.shout(&imheader, buffer.data(), sizeof(imheader), buffer.size(), group);
        }
        if (settings.image_viewer_on) {
            cv::imshow("Viewer", im);
            cv::waitKey(1);
        }
        
        if (settings.debug_mode_on) {
            // print out
            std::vector<int> detected;
            for (uint k=0; k<robots.size(); k++) {
                if (robots[k]->detected()) {
                    detected.push_back(robots[k]->code());
                }
            }
            std::cout << "Detected " << detected.size() << " robot(s) ";
            std::cout << "[";
            for (uint k=0; k<detected.size(); k++) {
                std::cout << detected[k];
                if (k+1 < detected.size()) {
                    std::cout << ",";
                }
            }
            std::cout << "]";
            std::cout << " and " << obstacles.size() << " obstacle(s)." << std::endl;
            std::cout << "t_cap: " << t_cap << "s - t_det: " << t_det << "s - t_com: " << t_com << "s" << std::endl;
        }
    }

    // stop the program
    cam->stop();
    delete cam;
    com.leave(group);
    com.stop();
    return 0;

}
