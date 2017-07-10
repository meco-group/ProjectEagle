//
// Created by peter on 06/07/17.
//

#ifndef PROJECTEAGLE_CALIBRATION_SETTINGS_H
#define PROJECTEAGLE_CALIBRATION_SETTINGS_H

#include "board_settings.h"

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

class CalSettings {
    public:
        // Was the input parsed correctly
        bool goodInput;

        // The source of the images_ceil1
        enum SourceType {NONE, STORED, VIDEO, CAMERA};
        SourceType sourceType;

        // Amount of frames to take
        int imageCount;

        // Source path;
        string sourcePath;

        // Data for case of STORED
        vector<string> imageList;

        // Board settings path
        string boardSettingsPath;

        // The board settings
        BoardSettings boardSettings;

        // The Calibration Flag
        int flag;

        // The output file name
        string outputFileName;

        // The aspect ratio
        float aspectRatio;

        // Flags
        bool calibZeroTangentDist;
        bool calibFixPrincipalPoint;

    private:
        string sourceTypeString;

    public:
        /**
         * Initialise this new CalSettings object
         */
        CalSettings();

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

static void read(const FileNode& node, CalSettings& x, const CalSettings& default_value = CalSettings()) {
    if(node.empty())
        x = default_value;
    else
        x.read(node);
}


#endif //PROJECTEAGLE_CALIBRATION_SETTINGS_H
