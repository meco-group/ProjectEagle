#include <eagle.h>
#include <unistd.h>

using namespace eagle;

int main(int argc, char* argv[]) {
    // parse arguments
    std::string iface = (argc > 1) ? argv[1] : "wlan0";
    std::string group = (argc > 2) ? argv[2] : "EAGLE";
    std::string peer = (argc > 3) ? argv[3] : "all";

    // start communicator
    Communicator com("receiver", iface, 5670);
    com.start(100.);
    usleep(1000000);
    com.join(group);

    bool receive_all = (peer.compare("all") == 0);
    std::vector<std::string> peers;

    // send streaming command to all devices in eagle
    header_t header = {CMD, 0}; //set time
    cmd_t cmd = IMAGE_STREAM_ON;
    if (!com.shout(&header, &cmd, sizeof(header), sizeof(cmd), group)) {
        std::cout << "communicator was not able to shout to " << group << std::endl;
        return -1;
    }

    std::cout << "Start receiving video stream." << std::endl;
    std::cout << "Press any key to exit" << std::endl;

    // start acquiring video stream
    std::string window_name = "";
    int img_cnt = 0;
    auto begin = std::chrono::system_clock::now();
    Message msg;
    while ( !kbhit() ) {
        if (com.listen(1)) { // listen for messages
            while(com.pop_message(msg)) { // read message queue
                if (receive_all || msg.peer().compare(peer) == 0) {
                    auto pnr = std::find(peers.begin(), peers.end(), msg.peer());
                    window_name = "Stream_";
                    if (pnr == peers.end()) {
                        peers.push_back(msg.peer());
                        window_name.append(std::to_string(peers.size()-1));
                    } else {
                        window_name.append(std::to_string(std::distance(peers.begin(), pnr)));
                    }
                    while (msg.available()) { // read frames in message
                        // 1. read the header
                        msg.read(&header);
                        // 2. read data based on the header
                        if (header.id == eagle::IMAGE) {
                            size_t size = msg.framesize();
                            uchar buffer[size];
                            msg.read(buffer);
                            cv::Mat rawData = cv::Mat( 1, size, CV_8UC1, buffer);
                            cv::Mat im = cv::imdecode(rawData, 1);
                            imshow(window_name, im);
                            cv::waitKey(1);
                            img_cnt++;
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

    // send streaming off command to all devices in eagle
    header = {CMD, 0}; //set time
    cmd = IMAGE_STREAM_OFF;
    if (!com.shout(&header, &cmd, sizeof(header), sizeof(cmd), group)) {
        std::cout << "communicator was not able to shout to " << group << std::endl;
        return -1;
    }

    // stop the program
    cvDestroyAllWindows();
    com.leave("EAGLE");
    com.stop();
    return 0;
}

