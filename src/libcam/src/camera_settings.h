//
// Created by peter on 07/07/17.
//

#ifndef PROJECTEAGLE_CAMERA_SETTINGS_H
#define PROJECTEAGLE_CAMERA_SETTINGS_H

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <libcam.hpp>
#include <libconfig.hpp>

namespace cam {

    using namespace cv;
    using namespace std;
    using namespace conf;

    class CameraSettings : public Config {
    public:
        int camIndex;
        CamType camType;
        string calPath;
        string extPath;
        string bgPath;
        string detPath;
        int res_width;
        int res_height;
        int comp_res_width;
        int comp_res_height;

    public:
        /**
         * Initialise this new CameraSettings object
         */
        CameraSettings();

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
         */
        bool parse();

    };
};


#endif //PROJECTEAGLE_CAMERA_SETTINGS_H
