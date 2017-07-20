#ifndef PROJECTEAGLE_BOARD_SETTINGS_H
#define PROJECTEAGLE_BOARD_SETTINGS_H

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <libconfig.hpp>

using namespace cv;
using namespace std;
using namespace conf;

class BoardSettings : public Config {
    public:
        enum Pattern { NOT_EXISTING, CHESSBOARD, CIRCLES_GRID, ASYMMETRIC_CIRCLES_GRID };

        Size boardSize;                 // The size of the board -> Number of items by width and height
        float squareSize;               // The size of a square in your defined unit (point, millimeter,etc).
        Pattern calibrationPattern;

    private:
        string patternToUse;

    public:
        /**
         * Initialise this new BoardSettings object
         */
        BoardSettings();

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


#endif //PROJECTEAGLE_BOARD_SETTINGS_H
