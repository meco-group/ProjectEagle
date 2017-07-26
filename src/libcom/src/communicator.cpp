#include "communicator.h"

using namespace com;

Communicator::Communicator(const std::string& name, const std::string& iface, int port) :
    _name(name) {
    _node = zyre_new(name.c_str());
    zyre_set_port(_node, port);
    zyre_set_interface(_node, iface.c_str());
}

Communicator::Communicator(const std::string& name, const std::string& iface) :
    _name(name) {
    _node = zyre_new(name.c_str());
    zyre_set_interface(_node, iface.c_str());
}

Communicator::Communicator(const std::string& name) : _name(name) {
    _node = zyre_new(name.c_str());
}

bool Communicator::start() {
    if (zyre_start(_node) != 0) {
        return false;
    }
    _poller = zpoller_new(zyre_socket(_node));
    return true;
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

void Communicator::debug() {
    zyre_set_verbose(_node);
}

std::vector<std::string> Communicator::mygroups() {
    std::vector<std::string> groups(0);
    zlist_t* group_lst = zyre_own_groups(_node);
    void* data = zlist_next(group_lst);
    while (data != NULL) {
        groups.push_back(std::string(static_cast<char*>(data)));
        data = zlist_next(group_lst);
    }
    zlist_destroy(&group_lst);
    return groups;
}

std::vector<std::string> Communicator::allgroups() {
    std::vector<std::string> groups(0);
    zlist_t* group_lst = zyre_peer_groups(_node);
    void* data = zlist_next(group_lst);
    while (data != NULL) {
        groups.push_back(std::string(static_cast<char*>(data)));
        data = zlist_next(group_lst);
    }
    zlist_destroy(&group_lst);
    return groups;
}

std::vector<std::string> Communicator::peers() {
    std::vector<std::string> peers(0);
    zlist_t* peer_lst = zyre_peers(_node);
    void* data = zlist_next(peer_lst);
    while (data != NULL) {
        peers.push_back(std::string(static_cast<char*>(data)));
        data = zlist_next(peer_lst);
    }
    zlist_destroy(&peer_lst);
    return peers;
}

std::vector<std::string> Communicator::peers(const std::string& group) {
    std::vector<std::string> peers(0);
    zlist_t* peer_lst = zyre_peers_by_group(_node, group.c_str());
    void* data = zlist_next(peer_lst);
    while (data != NULL) {
        peers.push_back(std::string(static_cast<char*>(data)));
        data = zlist_next(peer_lst);
    }
    zlist_destroy(&peer_lst);
    return peers;
}

bool Communicator::shout(const std::string& header, const void* data,
    size_t size, const std::string& group) {
    return shout(header, data, size, std::vector<std::string>{group});
}

bool Communicator::shout(const std::string& header, const void* data,
    size_t size, const std::vector<std::string>& groups) {
    return shout(header.c_str(), data, strlen(header.c_str())+1, size, groups);
}

bool Communicator::shout(const void* header, const void* data,
    size_t hsize, size_t dsize, const std::string& group) {
    return shout(header, data, hsize, dsize, std::vector<std::string>{group});
}

bool Communicator::shout(const void* header, const void* data,
    size_t hsize, size_t dsize, const std::vector<std::string>& groups) {
    return shout(std::vector<const void*>{header, data}, std::vector<size_t>{hsize, dsize}, groups);
}

bool Communicator::shout(const std::vector<const void*>& data,
    const std::vector<size_t>& sizes, const std::string& group) {
    return shout(data, sizes, std::vector<std::string>({group}));
}

bool Communicator::shout(const std::vector<const void*>& data,
    const std::vector<size_t>& sizes, const std::vector<std::string>& groups) {
    zmsg_t* msg = pack(data, sizes);
    for (int i=0; i<groups.size(); i++) {
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
    return whisper(header, data, size, std::vector<std::string>{peer});
}

bool Communicator::whisper(const std::string& header, const void* data,
    size_t size, const std::vector<std::string>& peers) {
    return whisper(header.c_str(), data, strlen(header.c_str())+1, size, peers);
}

bool Communicator::whisper(const void* header, const void* data,
    size_t hsize, size_t dsize, const std::string& peer) {
    return whisper(header, data, hsize, dsize, std::vector<std::string>{peer});
}

bool Communicator::whisper(const void* header, const void* data,
    size_t hsize, size_t dsize, const std::vector<std::string>& peers) {
    return whisper(std::vector<const void*>{header, data},
        std::vector<size_t>{hsize, dsize}, peers);
}

bool Communicator::whisper(const std::vector<const void*>& data,
    const std::vector<size_t>& sizes, const std::string& peer) {
    return whisper(data, sizes, std::vector<std::string>{peer});
}

bool Communicator::whisper(const std::vector<const void*>& data,
    const std::vector<size_t>& sizes, const std::vector<std::string>& peers) {
    zmsg_t* msg = pack(data, sizes);
    for (int i=0; i<peers.size(); i++) {
        if (zyre_whisper(_node, peers[i].c_str(), &msg) != 0) {
            zmsg_destroy(&msg);
            return false;
        }
    }
    zmsg_destroy(&msg);
    return true;
}

bool Communicator::receive(std::string& peer) {
    void* which = zpoller_wait(_poller, 0);
    if (which != zyre_socket(_node)) {
        return false;
    }
    _event = zyre_event_new(_node);
    const char* cmd = zyre_event_type(_event);
    if (streq(cmd, "SHOUT") || streq(cmd, "WHISPER")) {
        zmsg_t* msg = zyre_event_msg(_event);
        if (msg != NULL) {
            zframe_t* frame = zmsg_first(msg);
            _rcv_buffer = zframe_data(frame);
            _rcv_buffer_size = zframe_size(frame);
            _rcv_buffer_index = 0;
            peer = std::string(zyre_event_peer_name(_event));
        }
        return true;
    }
    return false;
}

bool Communicator::receive(void* header, void* data,
    size_t& hsize, size_t& dsize, std::string& peer) {
    if (!receive(peer)) {
        return false;
    }
    std::vector<void*> dat = {header, data};
    std::vector<size_t> sizes;
    read(2, dat, sizes);
    hsize = sizes[0];
    dsize = sizes[1];
    return true;
}

bool Communicator::receive(std::string& header, void* data,
    size_t& size, std::string& peer) {
    char header_pntr[1028];
    size_t hsize;
    if (!receive(header_pntr, data, hsize, size, peer)) {
        return false;
    }
    header = std::string(header_pntr);
    return true;
}

bool Communicator::listen(std::string& peer, double timeout) {
    clock_t t0 = timeout*CLOCKS_PER_SEC;
    clock_t t = clock();
    while (!receive(peer) && (t0<0 || (clock()-t) <= t0)) {};
    if (t0>=0 && (clock()-t) >= t0) {
        return false;
    }
    return true;
}

bool Communicator::listen(void* header, void* data,
    size_t& hsize, size_t& dsize, std::string& peer, double timeout) {
    clock_t t0 = timeout*CLOCKS_PER_SEC;
    clock_t t = clock();
    while (!receive(header, data, hsize, dsize, peer) && (t0<0 || (clock()-t) <= t0)) {};
    if (t0>=0 && (clock()-t) >= t0) {
        return false;
    }
    return true;
}

bool Communicator::listen(std::string& header, void* data,
    size_t& size, std::string& peer, double timeout) {
    clock_t t0 = timeout*CLOCKS_PER_SEC;
    clock_t t = clock();
    while (!receive(header, data, size, peer) && (t0<0 || (clock()-t) <= t0)) {};
    if (t0>=0 && (clock()-t) >= t0) {
        return false;
    }
    return true;
}

/* Low level packing and unpacking
 * Force 32 bit unsigned integer types for the packing as the size of size_t may differ depending on the architecture (32 vs 64 bit).
 */
typedef uint32_t communicator_size_t;

zmsg_t* Communicator::pack(const std::vector<const void*>& frames, const std::vector<size_t>& sizes) {
    size_t buffer_size = 0;
    for (int i=0; i<sizes.size(); i++) {
        buffer_size += sizeof(communicator_size_t);
        buffer_size += sizes[i];
    }
    void* buffer = malloc(buffer_size);
    unsigned int offset = 0;
    communicator_size_t size_caster;
    for (int i=0; i<frames.size(); i++) {
        size_caster = sizes[i];
        memcpy(buffer+offset, &size_caster, sizeof(communicator_size_t));
        offset += sizeof(communicator_size_t);
        memcpy(buffer+offset, frames[i], sizes[i]);
        offset += sizes[i];
    }
    zmsg_t* msg = zmsg_new();
    zmsg_pushmem(msg, buffer, buffer_size);
    free(buffer);
    return msg;
}

void Communicator::read(int n_frames, std::vector<void*>& frames, std::vector<size_t>& sizes) {
    bool check_size = (sizes.size() == frames.size());
    if (!check_size) {
        sizes.resize(frames.size(), 0);
    }
    for (int k=0; k<n_frames; k++) {
        if (k >= frames.size()) {
            break;
        }
        communicator_size_t size;
        if (available()) {
            memcpy(&size, _rcv_buffer+_rcv_buffer_index, sizeof(communicator_size_t));
            if (check_size && sizes[k] != 0 && sizes[k] != size) {
                std::cout << "Given size (" << sizes[k] <<
                ") does not match received frame size (" <<
                size << ")!" << std::endl;
                sizes[k] = (sizes[k] < size) ? sizes[k] : size;
            } else {
                sizes[k] = size;
            }
            _rcv_buffer_index += sizeof(communicator_size_t);
            memcpy(frames[k], _rcv_buffer+_rcv_buffer_index, sizes[k]);
            _rcv_buffer_index += sizes[k];
        } else {
            sizes[k] = 0;
        }
    }
}

void Communicator::read(void* frame) {
	int n_frames = 1;
	std::vector<void*> frames = {frame};
	std::vector<size_t> sizes = {0};
	read(n_frames, frames, sizes);
}

size_t Communicator::framesize() {
	communicator_size_t size;
	memcpy(&size, _rcv_buffer+_rcv_buffer_index, sizeof(communicator_size_t));
	return size;
}

bool Communicator::available() {
    return (_rcv_buffer_index < _rcv_buffer_size);
}