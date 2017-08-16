#include "calibration_settings.h"

using namespace eagle;

CalSettings::CalSettings() : Config("CalibrationSettings") {}

void CalSettings::write(FileStorage &fs) const {
    const char *sourceStrings[] = {"NONE", "STORED", "VIDEO", "CAMERA"};
    string str(sourceStrings[sourceType]);
    fs << _nodeName << "{"
       << "Image_Count"  << imageCount
       << "Source" << sourcePath
       << "Source_Type" << str
       << "Board_Settings" << boardSettingsPath
       << "Calibrate_FixAspectRatio" << aspectRatio
       << "Calibrate_AssumeZeroTangentialDistortion" << calibZeroTangentDist
       << "Calibrate_FixPrincipalPointAtTheCenter" << calibFixPrincipalPoint
       << "Output_File" << outputFileName
       << "}";
}

void CalSettings::read(const FileNode &node) {
    node["Image_Count" ] >> imageCount;
    node["Source"] >> sourcePath;
    node["Source_Type"] >> sourceTypeString;
    node["Board_Settings"]  >> boardSettingsPath;
    node["Calibrate_FixAspectRatio"]  >> aspectRatio;
    node["Calibrate_AssumeZeroTangentialDistortion"]  >> calibZeroTangentDist;
    node["Calibrate_FixPrincipalPointAtTheCenter"]  >> calibFixPrincipalPoint;
    node["Output_File"]  >> outputFileName;
    parse();
}

bool CalSettings::parse() {
    sourceType = NONE;
    if (!sourceTypeString.compare("STORED")) sourceType = STORED;
    if (!sourceTypeString.compare("VIDEO")) sourceType = VIDEO;
    if (!sourceTypeString.compare("CAMERA")) sourceType = CAMERA;
    if (sourceType == NONE) {
        cerr << " Inexistent camera calibration mode: " << sourceTypeString << endl;
        return false;
    }

    int flag = 0;
    if(calibFixPrincipalPoint) flag |= CV_CALIB_FIX_PRINCIPAL_POINT;
    if(calibZeroTangentDist)   flag |= CV_CALIB_ZERO_TANGENT_DIST;
    if(aspectRatio)            flag |= CV_CALIB_FIX_ASPECT_RATIO;
    this->flag = flag;

    // Open settings file TODO check file (see cal example)
    boardSettings.read(boardSettingsPath);

    // Open source path TODO check file (see cal example)
    if (sourceType == STORED) {
        FileStorage fs2(sourcePath, FileStorage::READ);
        FileNode n = fs2.getFirstTopLevelNode();
        FileNodeIterator it = n.begin(), it_end = n.end();
        for( ; it != it_end; ++it )
            imageList.push_back((std::string)*it);
    }

    return true;
}
