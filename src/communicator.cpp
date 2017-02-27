#include "communicator.h"

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

bool Communicator::join(const std::string& group) {
    if (zyre_join(_node, group.c_str())) {
        return false;
    }
    addToGroup(group, _name);
    return true;
}

bool Communicator::leave(const std::string& group) {
    if (zyre_leave(_node, group.c_str())) {
        return false;
    }
    removeFromGroup(group, _name);
    return true;
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
    zlist_t* peer_lst = zyre_peer_groups(_node);
    void* data = zlist_next(peer_lst);
    while (data != NULL) {
        peers.push_back(std::string(static_cast<char*>(data)));
        data = zlist_next(peer_lst);
    }
    zlist_destroy(&peer_lst);
    return peers;
}


bool Communicator::shout(const std::string& header, const void* data, size_t size, const std::string& group) {
    return shout(header, data, size, std::vector<std::string>{group});
}

bool Communicator::shout(const std::string& header, const void* data, size_t size, const std::vector<std::string>& groups) {
    return shout(std::vector<const void*>{header.c_str(), data}, std::vector<size_t>{strlen(header.c_str())+1, size}, groups);
}

bool Communicator::shout(const std::vector<const void*>& data, const std::vector<size_t>& sizes, const std::string& group) {
    return shout(data, sizes, std::vector<std::string>({group}));
}

bool Communicator::shout(const std::vector<const void*>& data, const std::vector<size_t>& sizes, const std::vector<std::string>& groups) {
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

bool Communicator::whisper(const std::string& header, const void* data, size_t size, const std::string& peer) {
    return whisper(header, data, size, std::vector<std::string>{peer});
}

bool Communicator::whisper(const std::string& header, const void* data, size_t size, const std::vector<std::string>& peers) {
    return whisper(std::vector<const void*>{header.c_str(), data}, std::vector<size_t>{strlen(header.c_str())+1, size}, peers);
}

bool Communicator::whisper(const std::vector<const void*>& data, const std::vector<size_t>& sizes, const std::string& peer) {
    return whisper(data, sizes, std::vector<std::string>{peer});
}

bool Communicator::whisper(const std::vector<const void*>& data, const std::vector<size_t>& sizes, const std::vector<std::string>& peers) {
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

bool Communicator::receive(zmsg_t* msg, std::string& peer) {
    void* which = zpoller_wait(_poller, 0);
    if (which != zyre_socket(_node)) {
        return false;
    }
    zyre_event_t* event = zyre_event_new(_node);
    const char* cmd = zyre_event_type(event);
    if (streq(cmd, "SHOUT") || streq(cmd, "WHISPER")) {
        msg = zyre_event_msg(event);
        peer = std::string(zyre_event_peer_name(event));
        zyre_event_destroy(&event);
        return true;
    }
    zyre_event_destroy(&event);
    return false;
}

bool Communicator::receive(std::vector<const void*>& data, std::vector<size_t>& sizes, std::string& peer) {
    zmsg_t* msg;
    if (!receive(msg, peer)) {
        zmsg_destroy(&msg);
        return false;
    }
    if (!unpack(msg, data, sizes)) {
        zmsg_destroy(&msg);
        return false;
    }
    zmsg_destroy(&msg);
    return true;
}

bool Communicator::receive(std::string& header, const void* data, size_t& size, std::string& peer) {
    std::vector<const void*> dat;
    std::vector<size_t> sizes;
    if (!receive(dat, sizes, peer)) {
        return false;
    }
    if (dat.size() == 2) {
        header = std::string(static_cast<const char*>(dat[0]));
        data = dat[1];
        size = sizes[1];
        return true;
    }
    return false;
}

bool Communicator::listen(zmsg_t* msg, std::string& peer) {
    while (!receive(msg, peer)) {};
    return (msg != NULL);
}

bool Communicator::listen(std::vector<const void*>& data, std::vector<size_t>& sizes, std::string& peer) {
    while (!receive(data, sizes, peer)) {};
    return true;
}

zmsg_t* Communicator::pack(const std::vector<const void*>& frames,
    const std::vector<size_t>& sizes) {
    zmsg_t* msg = zmsg_new();
    for (int i=0; i<frames.size(); i++) {
        zmsg_pushmem(msg, frames[i], sizes[i]);
    }
    return msg;
}

bool Communicator::unpack(zmsg_t* msg, std::vector<const void*>& frames, std::vector<size_t>& sizes) {
    if (msg == NULL) {
        return false;
    }
    zframe_t* frame = zmsg_next(msg);
    std::vector<const void*> frames_(0);
    std::vector<size_t> sizes_(0);
    while (frame != NULL) {
        frames_.push_back(zframe_data(frame));
        sizes_.push_back(zframe_size(frame));
        frame = zmsg_next(msg);
    }
    frames = frames_;
    sizes = sizes_;
    return true;
}

bool Communicator::start() {
    if (zyre_start(_node) != 0) {
        return false;
    }
    zclock_sleep(100);
    _poller = zpoller_new(zyre_socket(_node));
    return true;
}

bool Communicator::stop() {
    zyre_stop(_node);
    zyre_destroy(&_node);
    zpoller_destroy(&_poller);
}

void Communicator::addToGroup(const std::string& group, const std::string& peer) {
    if (_groups.find(group) == _groups.end()) {
        _groups[group] = {peer};
    } else {
        if (std::find(_groups[group].begin(), _groups[group].end(), peer) == _groups[group].end()) {
            _groups[group].push_back(peer);
        }
    }
}

void Communicator::removeFromGroup(const std::string& group, const std::string& peer) {
    if (_groups.find(group) == _groups.end()) {
        return;
    }
    _groups[group].erase(std::remove(_groups[group].begin(), _groups[group].end(), peer),
        _groups[group].end());
    if (_groups[group].empty()) {
        _groups.erase(group);
    }
}
