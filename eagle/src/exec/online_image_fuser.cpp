#include <iostream>
#include <fstream>
#include <camera.h>
#include <fuse.h>
#include <communicator.h>
#include <protocol.h>
#include <utils.h>
#include <unistd.h>

using namespace eagle;

int main(int argc, char* argv[]) {
    std::string config_path = (argc > 1) ? argv[1] : CONFIG_PATH;
    std::string iface = "wlan0";
    int port = 5670;
    
    int pixels_per_meter = 150; //px/m
    cv::Size img_size = cv::Size(6, 6); //m 
    double height = 0.0;
    cv::Mat img = cv::Mat::zeros(img_size.height*pixels_per_meter, img_size.width*pixels_per_meter, CV_8UC3);
 
    // read main config
    std::ifstream fs(config_path);
    if(!fs.is_open()){
        std::cout << "could not open configuration file" << std::endl;
        return 0;
    }

    std::string line;
    std::map<std::string, std::string> config_map;
    std::getline(fs, line);
    int device_id = 0;
    while(std::getline(fs,line)) {
        int pos = line.find(" ");
        if(pos != std::string::npos) {
            config_map[line.substr(0,pos)] = "../../gui/config/dev" + std::to_string(device_id) + "_config.xml";
            std::cout << line.substr(0,pos) << std::endl;
            device_id++;
        }
    }

    // start communication node
    std::string group = "EAGLE";
    Communicator com("Fuser",iface,port);
    com.start(100);
    com.join(group);

    // sleep before sending
    usleep(500000); //half a second

    // send streaming command to all devices in eagle
    header_t header = {CMD, 0}; //set time
    cmd_t cmd = IMAGE_STREAM_ON;
    if(!com.shout(&header, &cmd, sizeof(header), sizeof(cmd), group)) {
        std::cout << "communicator was not able to shout to " << group << std::endl;
        return -1;
    }

    cv::namedWindow("Stream");
    cv::imshow("Stream",img);
    cv::waitKey(30);

    std::cout << "passed waitkey" << std::endl;
    std::string pr;

    while( !kbhit() ) {
        if (com.listen(pr, 1)) {
            std::cout << pr << std::endl;
            if (config_map.find(pr) != config_map.end()) {
                while (com.available()) {
                    // 1. read the header
                    com.read(&header);
                    // 2. read data based on the header
                    if (header.id == eagle::IMAGE) {
                        size_t size = com.framesize();
                        uchar buffer[size];
                        com.read(buffer);
                        cv::Mat rawData = cv::Mat( 1, size, CV_8UC1, buffer);
                        cv::Mat in = cv::imdecode(rawData, 1);
                        cv::Mat remapped;
                        remapinf(config_map[pr], in, remapped, pixels_per_meter, img_size, height);
                        replace(remapped, img, 0.3);
                        cv::imshow("Stream",img);
                        cv::waitKey(30);
                        break;
                    }
                }
            } else {
                std::cout << pr << " not found in configured peers.." << std::endl;
            }
        } else {
            std::cout << "no message.. :(" << std::endl;
        }
    }

    // send streaming command to all devices in eagle
    header = {CMD, 0}; //set time
    cmd = IMAGE_STREAM_OFF;
    if(!com.shout(&header, &cmd, sizeof(header), sizeof(cmd), group)) {
        std::cout << "communicator was not able to shout to " << group << std::endl;
        return -1;
    }
    usleep(500000); //half a second

    com.leave("EAGLE");
    com.stop();

    return 0;
}

