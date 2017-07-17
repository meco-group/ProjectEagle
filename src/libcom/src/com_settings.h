//
// Created by peter on 12/07/17.
//

#ifndef PROJECTEAGLE_COMM_SETTINGS_H
#define PROJECTEAGLE_COMM_SETTINGS_H

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <libconfig.hpp>

namespace com {
    using namespace cv;
    using namespace std;
    using namespace conf;


    class ComSettings : public Config {
    public:
        string interface;
        int init_wait_time;

    public:
        /**
         * Initialise this new CommSettings object
         */
        ComSettings();

        /**
         * Write serialization for this class
         * @param fs file handle to write to
         */
        void write(FileStorage &fs) const;

        /**
         * Read serialization for this clss
         * @param node the file node to read
         */
        void read(const FileNode &node);

        /**
         * Read serialization for this class
         * @note we have to include this line to make the method overloading work
         */
        using Config::read;

        /**
         * Parse the data extracted from a settings file
         * @post goodInput is true if the data is correct otherwise it is false
         */
        bool parse();

    };
};

#endif //PROJECTEAGLE_COMM_SETTINGS_H
