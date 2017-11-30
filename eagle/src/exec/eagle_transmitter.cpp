#include <eagle.h>
#include <string>
#include <chrono>
#include <ctime>

using namespace eagle;

struct settings_t {
    bool snapshot;
    bool background;
    bool image_stream_on;
    bool detection_on;
    bool debug_mode_on;
    bool calibration_on;
    bool image_viewer_on;
    bool recording_on;
};

void transmit_detected(Communicator& com, const std::vector<Robot*>& robots, const std::vector<Obstacle*>& obstacles, std::string group, unsigned long capture_time) {
    // pack robots
    size_t n_robots = 0;
    eagle::header_t mheaders[robots.size()];
    eagle::marker_t markers[robots.size()];
    for (int k = 0; k < robots.size(); k++) {
        if ( robots[k]->detected() ) {
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
    std::vector< size_t > sizes = std::vector< size_t >(2 * n_robots + 2 * obstacles.size(), 0);
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
    eagle::header_t header;

    eagle::Message msg;
    if (com.receive()) {
        while (com.pop_message(msg)) {
            while (msg.available()) {
                // 1. read the header
                msg.read(&header);
                size_t size = msg.framesize();
                uchar buffer[size];
                msg.read(buffer);
                // 2. read data based on the header
                if (header.id == eagle::CMD) {
                    eagle::cmd_t cmd;
                    cmd = *((eagle::cmd_t*)(buffer));
                    std::cout << "CMD [" << cmd << "] received from " << msg.peer() << std::endl;
                    switch (cmd) {
                    case SNAPSHOT: {
                        settings.snapshot = true;
                        break;
                    }
                    case BACKGROUND: {
                        settings.background = true;
                        break;
                    }
                    case IMAGE_STREAM_ON: {
                        settings.image_stream_on = true;
                        break;
                    }
                    case IMAGE_STREAM_OFF: {
                        settings.image_stream_on = false;
                        break;
                    }
                    case IMAGE_STREAM_TOGGLE: {
                        settings.image_stream_on = !settings.image_stream_on;
                        break;
                    }
                    case DETECTION_ON: {
                        settings.detection_on = true;
                        break;
                    }
                    case DETECTION_OFF: {
                        settings.detection_on = false;
                        break;
                    }
                    case DETECTION_TOGGLE: {
                        settings.detection_on = !settings.detection_on;
                        break;
                    }
                    case DEBUG_MODE_ON: {
                        settings.debug_mode_on = true;
                        break;
                    }
                    case DEBUG_MODE_OFF: {
                        settings.debug_mode_on = false;
                        break;
                    }
                    case DEBUG_MODE_TOGGLE: {
                        settings.debug_mode_on = !settings.debug_mode_on;
                        break;
                    }
                    case CALIBRATION_ON: {
                        settings.calibration_on = true;
                        break;
                    }
                    case CALIBRATION_OFF: {
                        settings.calibration_on = false;
                        break;
                    }
                    case CALIBRATION_TOGGLE: {
                        settings.calibration_on = !settings.calibration_on;
                        break;
                    }
                    case RECORD_ON: {
                        settings.recording_on = true;
                        break;
                    }
                    case RECORD_OFF: {
                        settings.recording_on = false;
                        break;
                    }
                    case RECORD_TOGGLE: {
                        settings.recording_on = !settings.recording_on;
                        break;
                    }
                    default: { std::cout << "unknown command." << std::endl; }
                    }
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
    settings.debug_mode_on = false;
    settings.snapshot = false;
    settings.background = false;
    settings.recording_on = false;

    /******************/
    /* INITIALIZATION */
    /******************/

    std::cout << "Initialization started!" << std::endl;

    // read config file
    cv::FileStorage fs(CONFIG_PATH, cv::FileStorage::READ);
    std::string group = fs["communicator"]["group"];
    int zyre_wait_time = fs["communicator"]["zyre_wait_time"];
    std::string background_path = fs["detector"]["background_path"];
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

    // setup video recording
    cv::VideoWriter recorder;

    // robots and obstacles that the detector should search for
    Robot dave(0, 0.55, 0.4, cv::Scalar(138, 110, 17));
    Robot krist(1, 0.55, 0.4, cv::Scalar(17, 31, 138));
    Robot kurt(9, 0.55, 0.4, cv::Scalar(19, 138, 17));
    // Robot table0(2, 1.1, 1.2, cv::Point3f(-0.4705, -0.4175, 0.), cv::Scalar(64, 64, 169));
    // Robot table1(8, 1.1, 1.2, cv::Point3f(-0.4705, -0.4175, 0.), cv::Scalar(64, 64, 169));
    Robot table0(2, 1.1, 1.1, cv::Point3f(-0.52, -0.405, 0.), cv::Scalar(64, 64, 169));
    Robot table1(8, 1.4, 0.7, cv::Point3f(-0.7, -0.5, 0.), cv::Scalar(64, 64, 169));
    std::vector< Robot* > robots = std::vector< Robot* > {&dave, &krist, &kurt, &table0, &table1};
    std::vector< Obstacle* > obstacles;

    if (settings.image_viewer_on) {
        cv::namedWindow("Viewer", cv::WINDOW_AUTOSIZE);
    }

    /*************/
    /* MAIN LOOP */
    /*************/

    std::cout << "Main loop started!" << std::endl;

    INIT_TIMING;

    eagle::header_t imheader;
    imheader.id = eagle::IMAGE;
    cv::Mat im;
    cam->read(im);

    int dt = int(1000. / update_frequency); //in milliseconds
    auto t0 = std::chrono::high_resolution_clock::now();
    double t_cap, t_det, t_com;
    unsigned long capture_time;

    cv::Mat background = cv::Mat(im.size().height, im.size().width, CV_32FC3, cv::Scalar(0, 0, 0));
    int bg_counter = 0;

    while ( !kbhit() ) {
        // process communication
        process_communication(com, settings);

        //  check time
        auto t = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(t - t0).count() < dt) {
            continue;
        }
        t0 = t;

        // capture image
        TIMING(cam->read(im); capture_time = timestamp(););
        t_cap = DURATION;

        if (settings.snapshot) {
            settings.snapshot = false;
            char filename[100];
            std::time_t t = std::time(NULL);
            std::strftime(filename, sizeof(filename), "/snapshot_%Y_%m_%d_%H_%M_%S.png", std::localtime(&t));
            cv::imwrite(OUTPUT_PATH + std::string(filename), im);

            std::cout << "Snapshot taken. (" << OUTPUT_PATH + std::string(filename) << ")" << std::endl;
        }

        if (settings.recording_on) {
            if (!recorder.isOpened()) {
                char filename[100];
                std::time_t t = std::time(NULL);
                std::strftime(filename, sizeof(filename), "%Y_%m_%d_%H_%M_%S.avi", std::localtime(&t));
                recorder.open(OUTPUT_PATH + std::string(filename), CV_FOURCC('M', 'J', 'P', 'G'), update_frequency, cv::Size(cam->getWidth(), cam->getHeight()), true);
                std::cout << "Recorder opened. (" << OUTPUT_PATH + std::string(filename) << ")" << std::endl;
            }
            recorder.write(im);
        } else {
            if (recorder.isOpened()) {
                recorder.release();
                std::cout << "Recorder closed." << std::endl;
            }
        }

        if (settings.background) {
            bg_counter++;
            cv::accumulate(im, background);
            if (bg_counter >= 50) {
                background /= bg_counter;
                background.convertTo(background, CV_8UC3);
                detector.set_background(background);
                cv::imwrite(background_path, im);
                std::cout << "New background ready." << std::endl;

                //re-init
                settings.background = false;
                background = cv::Mat(im.size().height, im.size().width, CV_32FC3, cv::Scalar(0, 0, 0));
                bg_counter = 0;
            }
        }

        if (settings.detection_on) {
            // detect the robots/obstacles
            TIMING(detector.search(im, robots, obstacles););
            t_det = DURATION;
            // send detected robots/obstacles
            TIMING(transmit_detected(com, robots, obstacles, group, capture_time););
            t_com = DURATION;
            if (settings.image_viewer_on || settings.image_stream_on) {
                im = detector.draw(im, robots, obstacles);
            }
        } else {
            if (settings.image_viewer_on || settings.image_stream_on) {
                detector.projection()->remap(im, im);
            }
        }

        if (settings.calibration_on) {
            int t;
            extractor.extract(im, cv::Point2f(0, 0), t, true);
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
            for (uint k = 0; k < robots.size(); k++) {
                if (robots[k]->detected()) {
                    detected.push_back(robots[k]->code());
                }
            }
            std::cout << "Detected " << detected.size() << " robot(s) ";
            std::cout << "[";
            for (uint k = 0; k < detected.size(); k++) {
                std::cout << detected[k];
                if (k + 1 < detected.size()) {
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
