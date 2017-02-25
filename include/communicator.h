#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#include <zyre.h>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

class Communicator {

private:
    zyre_t* _node;
    zpoller_t* _poller;
    std::string _name;
    std::map<std::string, std::vector<std::string> > _groups;

    void addToGroup(const std::string& group, const std::string& peer);
    void removeFromGroup(const std::string& group, const std::string& peer);
    zmsg_t* pack(const std::vector<const void*>& frames, const std::vector<size_t>& sizes);

public:
    Communicator(const std::string& name, const std::string& iface, int port);
    Communicator(const std::string& name, const std::string& iface);
    Communicator(const std::string& name);

    bool join(const std::string& group);
    bool leave(const std::string& group);
    bool start();
    bool stop();
    void debug();
    std::vector<std::string> mygroups();
    std::vector<std::string> allgroups();
    bool shout(zmsg_t* msg, const std::string& group);
    bool shout(const std::string& header, const void* data,
        size_t size, const std::string& group);
    bool shout(const std::string& header, const void* data,
        size_t size, const std::vector<std::string>& groups);
    bool whisper(zmsg_t* msg, const std::string& group);
    bool whisper(const std::string& header, const void* data,
        size_t size, const std::string& peer);
    bool whisper(const std::string& header, const void* data,
        size_t size, const std::vector<std::string>& peers);

};

#endif //COMMUNICATOR_H
