#include <iostream>
#include <fstream>
#include <eagle.h>
#include <unistd.h>

using namespace eagle;

int main(int argc, char* argv[]) {
    std::string config_path = (argc > 1) ? argv[1] : CONFIG_PATH;
    std::string iface = "wlan0";
    int port = 5670;

    int pixels_per_meter = 150; //px/m
    cv::Size img_size = cv::Size(6, 6); //m
    double height = 0.0;
    cv::Mat img = cv::Mat::zeros(img_size.height * pixels_per_meter, img_size.width * pixels_per_meter, CV_8UC3);

    // read main config
    std::ifstream fs(config_path);
    if (!fs.is_open()) {
        std::cout << "could not open configuration file" << std::endl;
        return 0;
    }

    std::string line;
    std::map<std::string, std::string> config_map;
    std::getline(fs, line);
    int device_id = 0;
    while (std::getline(fs, line)) {
        int pos = line.find(" ");
        if (pos != std::string::npos) {
            config_map[line.substr(0, pos)] = "../../gui/config/dev" + std::to_string(device_id) + "_config.xml";
            std::cout << line.substr(0, pos) << std::endl;
            device_id++;
        }
    }

    // start communication node
    std::string group = "EAGLE";
    Communicator com("Fuser", iface, port);
    com.start(100);
    com.join(group);

    // sleep before sending
    usleep(500000); //half a second

    // send streaming command to all devices in eagle
    header_t header = {CMD, 0}; //set time
    cmd_t cmd = IMAGE_STREAM_ON;
    if (!com.shout(&header, &cmd, sizeof(header), sizeof(cmd), group)) {
        std::cout << "communicator was not able to shout to " << group << std::endl;
        return -1;
    }

    cv::namedWindow("Stream");
    cv::imshow("Stream", img);
    cv::waitKey(30);

    std::cout << "passed waitkey" << std::endl;
    std::string pr;

    Message msg;
    while (!kbhit()) {
        if (com.listen(1)) {
            while (com.pop_message(msg)) {
                pr = msg.peer();
                std::cout << msg.peer() << std::endl;
                if (config_map.find(pr) != config_map.end()) {
                    while (msg.available()) {
                        // 1. read the header
                        msg.read(&header);
                        // 2. read data based on the header
                        if (header.id == eagle::IMAGE) {
                            size_t size = msg.framesize();
                            std::vector<uchar> buffer(size);
                            msg.read(buffer.data());

                            // retrieve metadata
                            image_t iminfo;
                            memcpy((uchar*)(&iminfo),buffer.data()+buffer.size()-sizeof(iminfo),sizeof(iminfo));
                            buffer.resize(size-sizeof(iminfo));
                            
                            // decode image
                            cv::Mat im = cv::imdecode(buffer, 1);
                            replace(im, cv::Point2f(iminfo.offset.x, iminfo.offset.y), img, 0.3);
                            break;
                        } else {
                            msg.dump_frame();
                        }
                    }
                } else {
                    std::cout << pr << " not found in configured peers.." << std::endl;
                }
            }
        } else {
            std::cout << "no message.. :(" << std::endl;
        }
        cv::imshow("Stream", img);
        cv::waitKey(30);
    }

    // send streaming command to all devices in eagle
    header = {CMD, 0}; //set time
    cmd = IMAGE_STREAM_OFF;
    if (!com.shout(&header, &cmd, sizeof(header), sizeof(cmd), group)) {
        std::cout << "communicator was not able to shout to " << group << std::endl;
        return -1;
    }
    usleep(500000); //half a second

    com.leave("EAGLE");
    com.stop();

    return 0;
}

