//
// Created by peter on 12/07/17.
//

#ifndef PROJECTEAGLE_COMM_SETTINGS_H
#define PROJECTEAGLE_COMM_SETTINGS_H

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;


class CommSettings {
public:
    bool goodInput;
    string interface;
    int init_wait_time;

public:
    /**
     * Initialise this new CommSettings object
     */
    CommSettings();

    /**
     * Write serialization for this class
     * @param fs file handle to write to
     */
    void write(FileStorage& fs) const;

    /**
     * Read serialization for this clss
     * @param node the file node to read
     */
    void read(const FileNode& node);

    /**
     * Parse the data extracted from a settings file
     * @post goodInput is true if the data is correct otherwise it is false
     */
    void parse();
};

static void read(const FileNode& node, CommSettings& x, const CommSettings& default_value = CommSettings()) {
    if(node.empty())
        x = default_value;
    else
        x.read(node);
}


#endif //PROJECTEAGLE_COMM_SETTINGS_H
