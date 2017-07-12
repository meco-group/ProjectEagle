#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#include <zyre.h>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <ctime>

class Communicator {

private:
    zyre_t* _node;
    zpoller_t* _poller;
    zyre_event_t* _event;
    std::string _name;
    std::map<std::string, std::vector<std::string> > _groups;
    void* _rcv_buffer;
    size_t _rcv_buffer_size;
    unsigned int _rcv_buffer_index;

    zmsg_t* pack(const std::vector<const void*>& frames, const std::vector<size_t>& sizes);

public:
    Communicator(const std::string& name, const std::string& iface, int port);
    Communicator(const std::string& name, const std::string& iface);
    Communicator(const std::string& name);

    std::string name();
    bool start();
    bool start(int sleep_time);
    bool stop();
    bool join(const std::string& group);
    bool leave(const std::string& group);
    void debug();
    std::vector<std::string> mygroups();
    std::vector<std::string> allgroups();
    std::vector<std::string> peers();
    std::vector<std::string> peers(const std::string& group);

    bool shout(const std::string& header, const void* data,
        size_t size, const std::string& group);
    bool shout(const std::string& header, const void* data,
        size_t size, const std::vector<std::string>& groups);
    bool shout(const void* header, const void* data,
        size_t hsize, size_t dsize, const std::string& group);
    bool shout(const void* header, const void* data,
        size_t hsize, size_t dsize, const std::vector<std::string>& groups);
    bool shout(const std::vector<const void*>& data,
        const std::vector<size_t>& sizes, const std::string& group);
    bool shout(const std::vector<const void*>& data,
        const std::vector<size_t>& sizes, const std::vector<std::string>& groups);
    bool whisper(const std::string& header, const void* data,
        size_t size, const std::string& peer);
    bool whisper(const std::string& header, const void* data,
        size_t size, const std::vector<std::string>& peers);
    bool whisper(const void* header, const void* data,
        size_t hsize, size_t dsize, const std::string& peer);
    bool whisper(const void* header, const void* data,
        size_t hsize, size_t dsize, const std::vector<std::string>& peers);
    bool whisper(const std::vector<const void*>& data,
        const std::vector<size_t>& sizes, const std::string& peer);
    bool whisper(const std::vector<const void*>& data,
        const std::vector<size_t>& sizes, const std::vector<std::string>& peers);

    // non-blocking
    bool receive(std::string& peer);
    bool receive(void* header, void* data, size_t& hsize, size_t& dsize, std::string& peer);
    bool receive(std::string& header, void* data, size_t& size, std::string& peer);

    // blocking
    bool listen(std::string& peer, double timeout=-1);
    bool listen(void* header, void* data,
        size_t& hsize, size_t& dsize, std::string& peer, double timeout=-1);
    bool listen(std::string& header, void* data,
        size_t& size, std::string& peer, double timeout=-1);

    void read(int n_frames, std::vector<void*>& frames, std::vector<size_t>& sizes);
	void read(void* frame);
	size_t framesize();
    bool available();
};

#endif //COMMUNICATOR_H
