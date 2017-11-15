#include <eagle.h>

using namespace eagle;

int main(int argc, char* argv[]) {
    // parse arguments
    std::string iface = (argc > 1) ? argv[1] : "wlan0";
    std::string group = (argc > 2) ? argv[2] : "EAGLE";
    std::string peer = (argc > 3) ? argv[3] : "eagle0";

    // start communicator
    Communicator com("receiver", iface, 5670);
    com.start(100.);
    com.join(group);

    // send streaming command to all devices in eagle
    header_t header = {CMD, 0}; //set time
    cmd_t cmd = IMAGE_STREAM_ON;
    if (!com.shout(&header, &cmd, sizeof(header), sizeof(cmd), group)) {
        std::cout << "communicator was not able to shout to " << group << std::endl;
        return -1;
    }

    // video stream setup
    cv::namedWindow("Stream");
    std::string pr;

    std::cout << "Start receiving video stream." << std::endl;

    // start acquiring video stream
    int img_cnt = 0;
    auto begin = std::chrono::system_clock::now();
    Message msg;
    while ( !kbhit() ) {
        if (com.listen(1)) { // listen for messages
            while(com.pop_message(msg)) { // read message queue
                if (msg.peer().compare(peer) == 0) {
                    while (msg.available()) { // read frames in message
                        // 1. read the header
                        msg.read(&header);
                        // 2. read data based on the header
                        if (header.id == eagle::IMAGE) {
                            size_t size = msg.framesize();
                            uchar buffer[size];
                            msg.read(buffer);
                            if (pr == peer) {
                                cv::Mat rawData = cv::Mat( 1, size, CV_8UC1, buffer);
                                cv::Mat im = cv::imdecode(rawData, 1);
                                imshow("Stream", im);
                                cv::waitKey(1);
                                img_cnt++;
                            }
                            break;
                        } else {
                            msg.dump_frame();
                        }
                    }
                }
            }
        }
    }
    auto end = std::chrono::system_clock::now();
    double fps = img_cnt / (std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() * 1e-6);
    std::cout << "Average framerate: " << fps << std::endl;
    // stop the program
    cv::destroyWindow("Stream");
    com.leave("EAGLE");
    com.stop();
    return 0;
}

