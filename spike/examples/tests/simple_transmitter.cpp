#include "comm/communicator.h"
#include <ctime>
#include <random>

int main(void) {
    Communicator com("host" , "wlan0");
    // com.debug();
    com.start();
    com.join("mygroup");

    int data_snd1 = 36;
    int data_snd2 = 28;

    std::vector< size_t > sizes = std::vector< size_t >(2,0);
    std::vector< const void* > data = std::vector< const void* >(sizes.size());

    sizes[0] = sizeof(data_snd1);
    sizes[1] = sizeof(data_snd2);
    data[0] = &data_snd1;
    data[1] = &data_snd2;

    com.shout(data, sizes, "mygroup");

    com.leave("mygroup");
    com.stop();
}
