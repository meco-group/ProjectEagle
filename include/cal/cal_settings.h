#ifndef PROJECTEAGLE_CAL_SETTINGS_H
#define PROJECTEAGLE_CAL_SETTINGS_H

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

class CalSettings {

    public:
        enum Pattern { NOT_EXISTING, CHESSBOARD, CIRCLES_GRID, ASYMMETRIC_CIRCLES_GRID };
        enum InputType {INVALID, CAMERA, VIDEO_FILE, IMAGE_LIST};

        bool goodInput;

        Size boardSize;                 // The size of the board -> Number of items by width and height
        Pattern calibrationPattern;     // One of the Chessboard, circles, or asymmetric circle pattern
         float squareSize;               // The size of a square in your defined unit (point, millimeter,etc).
        int nrFrames;                   // The number of frames to use from the input for calibration
        float aspectRatio;              // The aspect ratio
        int delay;                      // In case of a video input
        bool bwritePoints;              //  Write detected feature points
        bool bwriteExtrinsics;          // Write extrinsic parameters
        bool calibZeroTangentDist;      // Assume zero tangential distortion
        bool calibFixPrincipalPoint;    // Fix the principal point at the center
        bool flipVertical;              // Flip the captured images around the horizontal axis
        string outputFileName;          // The name of the file where to write
        bool showUndistorsed;           // Show undistorted images after calibration
        string input;                   // The input ->

        int cameraID;
        vector<string> imageList;
        int atImageList;
        VideoCapture inputCapture;
        InputType inputType;
        int flag;

    private:
        string patternToUse;

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
         * Read serialization for this class
         * @param node
         */
        void read(const FileNode& node);

        /**
         * Get the next image to use for calibration
         * @return an image stored in an OpenCV matrix
         */
        Mat nextImage();

    private:
        /**
         * Parse the data extracted from a settings file
         * @post goodInput is true if the data is correct otherwise it is false
         */
        void parse();

        /**
         * Read the image list file
         * @param filename path of image list file
         * @param l the resulting list
         * @return does the provided file contain a list of images
         */
        static bool readStringList( const string& filename, vector<string>& l );
};

#endif //PROJECTEAGLE_CAL_SETTINGS_H
