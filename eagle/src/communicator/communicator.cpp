#include "communicator.h"

using namespace eagle;

Communicator::Communicator(const std::string& name, const std::string& iface, int port) : _name(name), _verbose(0) {
    _node = zyre_new(name.c_str());
    zyre_set_port(_node, port);
    zyre_set_interface(_node, iface.c_str());
}

Communicator::Communicator(const std::string& name) : _name(name), _verbose(0) {
    _node = zyre_new(name.c_str());
}

Communicator::Communicator(const std::string& name, const std::string& config_path) : _name(name), _verbose(0) {
    cv::FileStorage fs(config_path, cv::FileStorage::READ);
    _node = zyre_new(name.c_str());
    zyre_set_port(_node, (int)fs["communicator"]["zyre_port"]);
    zyre_set_interface(_node, ((std::string)fs["communicator"]["interface"]).c_str());
}

bool Communicator::start() {
    return start(0);
}

bool Communicator::start(int sleep_time) {
    if (zyre_start(_node) != 0) {
        return false;
    }
    if (sleep_time > 0)
        zclock_sleep(sleep_time); // causes segmentation errors
    _poller = zpoller_new(zyre_socket(_node));
    return true;
}

bool Communicator::stop() {
    zyre_stop(_node);
    zyre_destroy(&_node);
    zpoller_destroy(&_poller);
}

bool Communicator::join(const std::string& group) {
    if (zyre_join(_node, group.c_str())) {
        return false;
    }
    _groups[group].push_back(_name);
    return true;
}

bool Communicator::leave(const std::string& group) {
    if (zyre_leave(_node, group.c_str())) {
        return false;
    }
    return true;
}

std::string Communicator::name() {
    return std::string(zyre_name(_node));
}

void Communicator::verbose(int verbose) {
    _verbose = verbose;
}

void Communicator::debug() {
    zyre_set_verbose(_node);
}

std::vector<std::string> Communicator::mygroups() {
    std::vector<std::string> groups;
    for (auto it = _groups.begin(); it != _groups.end(); it++) {
        auto loc = std::find(it->second.begin(), it->second.end(), _name);
        if (loc != it->second.end()) {
            groups.push_back(it->first);
        }
    }

    return groups;
}

std::vector<std::string> Communicator::allgroups() {
    std::vector<std::string> groups;
    for (auto it = _groups.begin(); it != _groups.end(); it++)
        groups.push_back(it->first);

    return groups;
}

std::vector<std::string> Communicator::peers() {
    std::vector<std::string> peers;
    for (auto it = _peers.begin(); it != _peers.end(); it++)
        peers.push_back(it->first);

    return peers;
}

std::vector<std::string> Communicator::peers(const std::string& group) {
    if (_groups.find(group) != _groups.end())
        return _groups[group];

    return std::vector<std::string>(0);
}

bool Communicator::shout(const std::string& header, const void* data,
                         size_t size, const std::string& group) {
    return shout(header, data, size, std::vector<std::string> {group});
}

bool Communicator::shout(const std::string& header, const void* data,
                         size_t size, const std::vector<std::string>& groups) {
    return shout(header.c_str(), data, strlen(header.c_str()) + 1, size, groups);
}

bool Communicator::shout(const void* header, const void* data,
                         size_t hsize, size_t dsize, const std::string& group) {
    return shout(header, data, hsize, dsize, std::vector<std::string> {group});
}

bool Communicator::shout(const void* header, const void* data,
                         size_t hsize, size_t dsize, const std::vector<std::string>& groups) {
    return shout(std::vector<const void*> {header, data}, std::vector<size_t> {hsize, dsize}, groups);
}

bool Communicator::shout(const std::vector<const void*>& data,
                         const std::vector<size_t>& sizes, const std::string& group) {
    return shout(data, sizes, std::vector<std::string>({group}));
}

bool Communicator::shout(const std::vector<const void*>& data,
                         const std::vector<size_t>& sizes, const std::vector<std::string>& groups) {
    zmsg_t* msg = pack(data, sizes);
    for (int i = 0; i < groups.size(); i++) {
        if (_verbose >= 1) {
            std::cout << "[" << _name << "] shouting to " << groups[i] << "." << std::endl;
        }
        if (zyre_shout(_node, groups[i].c_str(), &msg) != 0) {
            zmsg_destroy(&msg);
            return false;
        }
    }
    zmsg_destroy(&msg);
    return true;
}

bool Communicator::whisper(const std::string& header, const void* data,
                           size_t size, const std::string& peer) {
    return whisper(header, data, size, std::vector<std::string> {peer});
}

bool Communicator::whisper(const std::string& header, const void* data,
                           size_t size, const std::vector<std::string>& peers) {
    return whisper(header.c_str(), data, strlen(header.c_str()) + 1, size, peers);
}

bool Communicator::whisper(const void* header, const void* data,
                           size_t hsize, size_t dsize, const std::string& peer) {
    return whisper(header, data, hsize, dsize, std::vector<std::string> {peer});
}

bool Communicator::whisper(const void* header, const void* data,
                           size_t hsize, size_t dsize, const std::vector<std::string>& peers) {
    return whisper(std::vector<const void*> {header, data},
    std::vector<size_t> {hsize, dsize}, peers);
}

bool Communicator::whisper(const std::vector<const void*>& data,
                           const std::vector<size_t>& sizes, const std::string& peer) {
    return whisper(data, sizes, std::vector<std::string> {peer});
}

bool Communicator::whisper(const std::vector<const void*>& data,
                           const std::vector<size_t>& sizes, const std::vector<std::string>& peers) {
    zmsg_t* msg = pack(data, sizes);
    for (int i = 0; i < peers.size(); i++) {
        auto p = _peers.find(peers[i]);
        if (p != _peers.end()) {
            if (_verbose >= 1) {
                std::cout << "[" << _name << "] whispering to " << peers[i] << "." << std::endl;
            }
            if (zyre_whisper(_node, p->second.c_str(), &msg) != 0) {
                zmsg_destroy(&msg);
                return false;
            }
        } else {
            std::cout << "No uuid found for " << peers[i];
        }
    }
    zmsg_destroy(&msg);
    return true;
}

bool Communicator::receive() {
    void* which;
    bool data_received = false;
    while(true) {
        which = zpoller_wait(_poller, 0);
        if ((which != zyre_socket(_node)) || (which == NULL)) { // pipe is empty
            break;
        }
        _event = zyre_event_new(_node);
        const char* cmd = zyre_event_type(_event);
        std::string peer = std::string(zyre_event_peer_name(_event));
        if (streq(cmd, "SHOUT") || streq(cmd, "WHISPER")) {
            zmsg_t* msg = zyre_event_msg(_event);
            if (msg != NULL) {
                push_message(msg, peer);
                data_received = true;
            }
        } else if (streq(cmd, "ENTER")) {
            std::string uuid = std::string(zyre_event_peer_uuid(_event));
            _peers[peer] = uuid;
            if (_verbose >= 2) {
                std::cout << "[" << _name << "] peer " << peer << " entered the network." << std::endl;
            }
        } else if (streq(cmd, "JOIN")) {
            std::string group = std::string(zyre_event_group(_event));
            _groups[group].push_back(peer);
            if (_verbose >= 2) {
                std::cout << "[" << _name << "] peer " << peer << " joined group " << group << "." << std::endl;
            }
        } else if (streq(cmd, "EXIT")) {
            _peers.erase(peer);
            if (_verbose >= 2) {
                std::cout << "[" << _name << "] peer " << peer << " left the network." << std::endl;
            }
        } else if (streq(cmd, "LEAVE")) {
            std::string group = std::string(zyre_event_group(_event));
            auto loc = std::find(_groups[group].begin(), _groups[group].end(), peer);
            if (loc != _groups[group].end()) {
                _groups[group].erase(loc);
            }
            if (_groups[group].empty()) {
                _groups.erase(group);
            }
            if (_verbose >= 2) {
                std::cout << "[" << _name << "] peer " << peer << " left group " << group << "." << std::endl;
            }
        }
    }
    return data_received;
}

void Communicator::push_message(zmsg_t* msg, std::string& peer) {
    Message m(msg, peer);
    _messages.push(m);
    if (_verbose >= 1) {
        std::cout << "[" << _name << "] receiving from " << peer << "." << std::endl;
    }
}

bool Communicator::pop_message(Message& msg) {
    if (_messages.size() == 0) {
        return false;
    }
    msg = _messages.front();
    _messages.pop();
    return true;
}

bool Communicator::listen(double timeout) {
    clock_t t0 = timeout * CLOCKS_PER_SEC;
    clock_t t = clock();
    while (!receive() && (t0 < 0 || (clock() - t) <= t0)) {};
    if (t0 >= 0 && (clock() - t) >= t0) {
        return false;
    }
    return true;
}

bool Communicator::listen(void* header, void* data,
                          size_t& hsize, size_t& dsize, std::string& peer, double timeout) {
    clock_t t0 = timeout * CLOCKS_PER_SEC;
    clock_t t = clock();
    while (!receive() && (t0 < 0 || (clock() - t) <= t0)) {};
    if (t0 >= 0 && (clock() - t) >= t0) {
        return false;
    }
    std::vector<void*> dat = {header, data};
    std::vector<size_t> sizes;
    Message msg;
    if (pop_message(msg)) {
        msg.read(2, dat, sizes);
        peer = msg.peer();
        hsize = sizes[0];
        dsize = sizes[1];
        return true;
    } else {
        return false;
    }
}

bool Communicator::listen(std::string& header, void* data,
                          size_t& size, std::string& peer, double timeout) {
    char header_pntr[1028];
    size_t hsize;
    if (!listen(header_pntr, data, hsize, size, peer, timeout)) {
        return false;
    }
    header = std::string(header_pntr);
    return true;
}

zmsg_t* Communicator::pack(const std::vector<const void*>& frames, const std::vector<size_t>& sizes) {
    size_t buffer_size = 0;
    for (int i = 0; i < sizes.size(); i++) {
        buffer_size += sizeof(communicator_size_t);
        buffer_size += sizes[i];
    }
    void* buffer = malloc(buffer_size);
    unsigned int offset = 0;
    communicator_size_t size_caster;
    for (int i = 0; i < frames.size(); i++) {
        size_caster = sizes[i];
        memcpy(((uint8_t*)buffer) + offset, &size_caster, sizeof(communicator_size_t));
        offset += sizeof(communicator_size_t);
        memcpy(((uint8_t*)buffer) + offset, frames[i], sizes[i]);
        offset += sizes[i];
    }
    zmsg_t* msg = zmsg_new();
    zmsg_pushmem(msg, buffer, buffer_size);
    free(buffer);
    return msg;
}
