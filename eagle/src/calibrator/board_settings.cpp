#include "board_settings.h"

BoardSettings::BoardSettings() : Config("BoardSettings") {}

void BoardSettings::write(FileStorage &fs) const {
    const char *patternStrings[] = {"NOT_EXISTING", "CHESSBOARD", "CIRCLES_GRID", "ASSYMETRIC_CIRCLES_GRID"};
    string str(patternStrings[calibrationPattern]);
    fs << _nodeName << "{"
       << "BoardSize_Width"  << boardSize.width
       << "BoardSize_Height" << boardSize.height
       << "Square_Size" << squareSize
       << "Calibrate_Pattern" << str
       << "}";
}

void BoardSettings::read(const FileNode &node) {
    node["BoardSize_Width" ] >> boardSize.width;
    node["BoardSize_Height"] >> boardSize.height;
    node["Calibrate_Pattern"] >> patternToUse;
    node["Square_Size"]  >> squareSize;
    parse();
}

bool BoardSettings::parse() {
    if (boardSize.width <= 0 || boardSize.height <= 0) {
        cerr << "Invalid Board size: " << boardSize.width << " " << boardSize.height << endl;
        return false;
    }
    if (squareSize <= 10e-6) {
        cerr << "Invalid square size " << squareSize << endl;
        return false;
    }

    calibrationPattern = NOT_EXISTING;
    if (!patternToUse.compare("CHESSBOARD")) calibrationPattern = CHESSBOARD;
    if (!patternToUse.compare("CIRCLES_GRID")) calibrationPattern = CIRCLES_GRID;
    if (!patternToUse.compare("ASYMMETRIC_CIRCLES_GRID")) calibrationPattern = ASYMMETRIC_CIRCLES_GRID;
    if (calibrationPattern == NOT_EXISTING) {
        cerr << " Inexistent camera calibration mode: " << patternToUse << endl;
        return false;
    }

    return true;
}
