//
// Created by peter on 06/07/17.
//

#include "cal/calibrator.h"


Calibrator::Calibrator(CalSettings s) {
    _settings = s;
};

Mat Calibrator::getNextImage() {
    Mat result;
    // Check the source of the images
    switch(_settings.sourceType) {
        case CalSettings::SourceType::STORED:
            // We can get an image from the provided file
            if( _imageIndex < (int)_settings.imageList.size())
                result = imread(_settings.imageList[_imageIndex++], CV_LOAD_IMAGE_COLOR);
            break;
        default:
            break;
    }

    return result;
};

bool Calibrator::processImage(Mat view, vector<Point2f> &pointBuf) {
    // Flip the image if requested
    if( _settings.flipVertical )
        flip( view, view, 0 );

    // Process image based on format
    bool found;
    switch( _settings.boardSettings.calibrationPattern ) {
        case BoardSettings::CHESSBOARD:
            found = findChessboardCorners( view, _settings.boardSettings.boardSize, pointBuf,
                                           CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);
            break;
        case BoardSettings::CIRCLES_GRID:
            found = findCirclesGrid( view, _settings.boardSettings.boardSize, pointBuf );
            break;
        case BoardSettings::ASYMMETRIC_CIRCLES_GRID:
            found = findCirclesGrid( view, _settings.boardSettings.boardSize, pointBuf, CALIB_CB_ASYMMETRIC_GRID );
            break;
        default:
            found = false;
            break;
    }

    if (found) {
        // improve the found corners' coordinate accuracy for chessboard
        if( _settings.boardSettings.calibrationPattern == BoardSettings::CHESSBOARD) {
            Mat viewGray;
            cvtColor(view, viewGray, COLOR_BGR2GRAY);
            cornerSubPix( viewGray, pointBuf, Size(11,11),
                          Size(-1,-1), TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));
        }
    }

    return found;
};

bool Calibrator::processPattern(vector<Point3f> &pointBuf) {
    // Reset the corners
    pointBuf.clear();

    // Calculate grid points based on pattern
    switch(_settings.boardSettings.calibrationPattern) {
        case BoardSettings::CHESSBOARD:
        case BoardSettings::CIRCLES_GRID:
            for( int i = 0; i < _settings.boardSettings.boardSize.height; ++i )
                for( int j = 0; j < _settings.boardSettings.boardSize.width; ++j )
                    pointBuf.push_back(Point3f(float( j*_settings.boardSettings.squareSize ), float( i*_settings.boardSettings.squareSize ), 0));
            break;

        case BoardSettings::ASYMMETRIC_CIRCLES_GRID:
            for( int i = 0; i < _settings.boardSettings.boardSize.height; i++ )
                for( int j = 0; j < _settings.boardSettings.boardSize.width; j++ )
                    pointBuf.push_back(Point3f(float((2*j + i % 2)*_settings.boardSettings.squareSize), float(i*_settings.boardSettings.squareSize), 0));
            break;
        default:
            break;
    }
};
