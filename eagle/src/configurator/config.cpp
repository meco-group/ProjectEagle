// Config::
// Created by peter on 14/07/17.
//

#include "config.hpp"

conf::Config::Config(std::string nodeName) : _nodeName(nodeName) {}

void conf::Config::read(const string path) {
    FileStorage fs(path, FileStorage::READ);
    read(fs[_nodeName]);
    fs.release();
}