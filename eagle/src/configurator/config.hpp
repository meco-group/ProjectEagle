#ifndef PROJECTEAGLE_CONFIG_HPP
#define PROJECTEAGLE_CONFIG_HPP

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

namespace eagle {
    using namespace cv;
    using namespace std;

    class Config {
    protected:
        const string _nodeName;

    public:
        /**
         * Initialise this new Config object
         */
        Config(string nodeName);

        /**
         * Write serialization for this class
         * @param fs file handle to write to
         */
        virtual void write(FileStorage& fs) const = 0;

        /**
         * Read serialization for this clss
         * @param node the file node to read
         */
        virtual void read(const FileNode& node)= 0;

        /**
         * Read serialization for this clss
         * @param node the file to read
         */
        void read(const string path);

        /**
         * Parse the data extracted from a settings file
         */
        virtual bool parse()= 0;

    };
}


#endif //PROJECTEAGLE_CONFIG_HPP
