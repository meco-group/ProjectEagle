#include <cal/cal_settings.h>

using namespace cv;
using namespace std;

CalSettings::CalSettings() : goodInput(false) {}

void CalSettings::write(FileStorage &fs) const {
    fs << "{" << "BoardSize_Width"  << boardSize.width
       << "BoardSize_Height" << boardSize.height
       << "Square_Size" << squareSize
       << "Calibrate_Pattern" << patternToUse
       << "Calibrate_NrOfFrameToUse" << nrFrames
       << "Calibrate_FixAspectRatio" << aspectRatio
       << "Calibrate_AssumeZeroTangentialDistortion" << calibZeroTangentDist
       << "Calibrate_FixPrincipalPointAtTheCenter" << calibFixPrincipalPoint

       << "Write_DetectedFeaturePoints" << bwritePoints
       << "Write_extrinsicParameters" << bwriteExtrinsics
       << "Write_outputFileName" << outputFileName

       << "Show_UndistortedImage" << showUndistorsed

       << "Input_FlipAroundHorizontalAxis" << flipVertical
       << "Input_Delay" << delay
       << "Input" << input
       << "}";
}

void CalSettings::read(const FileNode &node) {
    node["BoardSize_Width" ] >> boardSize.width;
    node["BoardSize_Height"] >> boardSize.height;
    node["Calibrate_Pattern"] >> patternToUse;
    node["Square_Size"]  >> squareSize;
    node["Calibrate_NrOfFrameToUse"] >> nrFrames;
    node["Calibrate_FixAspectRatio"] >> aspectRatio;
    node["Write_DetectedFeaturePoints"] >> bwritePoints;
    node["Write_extrinsicParameters"] >> bwriteExtrinsics;
    node["Write_outputFileName"] >> outputFileName;
    node["Calibrate_AssumeZeroTangentialDistortion"] >> calibZeroTangentDist;
    node["Calibrate_FixPrincipalPointAtTheCenter"] >> calibFixPrincipalPoint;
    node["Input_FlipAroundHorizontalAxis"] >> flipVertical;
    node["Show_UndistortedImage"] >> showUndistorsed;
    node["Input"] >> input;
    node["Input_Delay"] >> delay;
    parse();
}

Mat CalSettings::nextImage() {
    Mat result;
    if( inputCapture.isOpened()) {
        Mat view0;
        inputCapture >> view0;
        view0.copyTo(result);
    }
    else if( atImageList < (int)imageList.size())
        result = imread(imageList[atImageList++], CV_LOAD_IMAGE_COLOR);

    return result;
}

void CalSettings::parse() {
    goodInput = true;
    if (boardSize.width <= 0 || boardSize.height <= 0)
    {
        cerr << "Invalid Board size: " << boardSize.width << " " << boardSize.height << endl;
        goodInput = false;
    }
    if (squareSize <= 10e-6)
    {
        cerr << "Invalid square size " << squareSize << endl;
        goodInput = false;
    }
    if (nrFrames <= 0)
    {
        cerr << "Invalid number of frames " << nrFrames << endl;
        goodInput = false;
    }

    if (input.empty())      // Check for valid input
        inputType = INVALID;
    else {
        if (input[0] >= '0' && input[0] <= '9') {
            stringstream ss(input);
            ss >> cameraID;
            inputType = CAMERA;
        } else {
            if (readStringList(input, imageList)) {
                inputType = IMAGE_LIST;
                nrFrames = (nrFrames < (int)imageList.size()) ? nrFrames : (int)imageList.size();
            } else
                inputType = VIDEO_FILE;
        }
        if (inputType == CAMERA)
            inputCapture.open(cameraID);
        if (inputType == VIDEO_FILE)
            inputCapture.open(input);
        if (inputType != IMAGE_LIST && !inputCapture.isOpened())
            inputType = INVALID;
    }
    if (inputType == INVALID) {
        cerr << " Inexistent input: " << input;
        goodInput = false;
    }

    flag = 0;
    if(calibFixPrincipalPoint) flag |= CV_CALIB_FIX_PRINCIPAL_POINT;
    if(calibZeroTangentDist)   flag |= CV_CALIB_ZERO_TANGENT_DIST;
    if(aspectRatio)            flag |= CV_CALIB_FIX_ASPECT_RATIO;


    calibrationPattern = NOT_EXISTING;
    if (!patternToUse.compare("CHESSBOARD")) calibrationPattern = CHESSBOARD;
    if (!patternToUse.compare("CIRCLES_GRID")) calibrationPattern = CIRCLES_GRID;
    if (!patternToUse.compare("ASYMMETRIC_CIRCLES_GRID")) calibrationPattern = ASYMMETRIC_CIRCLES_GRID;
    if (calibrationPattern == NOT_EXISTING) {
        cerr << " Inexistent camera calibration mode: " << patternToUse << endl;
        goodInput = false;
    }
    atImageList = 0;
}

bool CalSettings::readStringList(const string &filename, vector<string> &l) {
    l.clear();
    FileStorage fs(filename, FileStorage::READ);
    if( !fs.isOpened() )
        return false;
    FileNode n = fs.getFirstTopLevelNode();
    if( n.type() != FileNode::SEQ )
        return false;
    FileNodeIterator it = n.begin(), it_end = n.end();
    for( ; it != it_end; ++it )
        l.push_back((std::string)*it);
    return true;
}