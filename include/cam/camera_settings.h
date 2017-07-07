//
// Created by peter on 07/07/17.
//

#ifndef PROJECTEAGLE_CAMERA_SETTINGS_H
#define PROJECTEAGLE_CAMERA_SETTINGS_H

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <cam/libcam.h>

using namespace cv;
using namespace std;

class CameraSettings {
    public:
        bool goodInput;
        int camIndex;
        CamType camType;
        string calPath;

    public:
        /**
         * Initialise this new CameraSettings object
         */
        CameraSettings();

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

static void read(const FileNode& node, CameraSettings& x, const CameraSettings& default_value = CameraSettings()) {
    if(node.empty())
        x = default_value;
    else
        x.read(node);
}


#endif //PROJECTEAGLE_CAMERA_SETTINGS_H
