#include "cal/board_settings.h"
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

void gen_settings() {
    // Generate the settings file
    BoardSettings s;
    s.boardSize = Size(9, 6);
    s.squareSize = 25;
    s.calibrationPattern = s.CHESSBOARD;

    // Open and write file
    FileStorage fs("test.xml", FileStorage::WRITE);
    s.write(fs);
    fs.release();
}

void cal_test() {
    // Open settings file
    FileStorage fs("board_settings.xml", FileStorage::READ);
    BoardSettings s;
    fs["BoardSettings"] >> s;
    fs.release();
}