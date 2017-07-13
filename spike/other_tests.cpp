//
// Created by peter on 12/07/17.
//

#include "experiments.h"

void solvepnp() {
    // Open settings file
    FileStorage fs1("../config/ceil2_cam.xml", FileStorage::READ);
    CalSettings calSettings;
    fs1["CalibrationSettings"] >> calSettings;
    fs1.release();

    // Get the intrinsic matrices
    Mat camMatrix; Mat distMatrix;
    cv::FileStorage fs2(calSettings.outputFileName, cv::FileStorage::READ);
    fs2["camera_matrix"] >> camMatrix;
    fs2["distortion_vector"] >> distMatrix;
    fs2.release();

    // Load image
    Mat im = imread("../config/ceil2_images/5.jpg", CV_LOAD_IMAGE_COLOR);

    imshow("Image View", im);
    (char)waitKey(1000);

    // Calculate the corners
    vector<Point2f> imagePoints;

    findChessboardCorners( im, calSettings.boardSettings.boardSize, imagePoints, 0);
    Mat viewGray;
    cvtColor(im, viewGray, COLOR_BGR2GRAY);
    cornerSubPix( viewGray, imagePoints, calSettings.boardSettings.boardSize,
                  Size(-1,-1), TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));

    // Draw the corners.
    Mat view = im.clone();
    drawChessboardCorners( view, calSettings.boardSettings.boardSize, Mat(imagePoints), true );

    imshow("Image View", view);
    (char)waitKey(1000);

    cout << getOptimalNewCameraMatrix(camMatrix, distMatrix, view.size(), 1, view.size(), 0) << "\n";

    Mat rview, map1, map2;
    initUndistortRectifyMap(camMatrix, distMatrix, Mat(), camMatrix, view.size(), CV_16SC2, map1, map2);
    remap(im, rview, map1, map2, INTER_LINEAR);
    imshow("Undistorted", rview);
    (char)waitKey(5000);
    // Store snapshot
    cv::imwrite("snapshot2.png", rview);


    // Get the objectPoints
    vector<Point3f> objectPoints;
    for( int i = 0; i < calSettings.boardSettings.boardSize.height; ++i )
        for( int j = 0; j < calSettings.boardSettings.boardSize.width; ++j )
            objectPoints.push_back(Point3f(float( j*calSettings.boardSettings.squareSize ), float( i*calSettings.boardSettings.squareSize ), 0));

    Mat rvec; Mat tvec;
    solvePnP(objectPoints, imagePoints, camMatrix, distMatrix, rvec, tvec);

    cout << rvec << "\n";
    cout << tvec << "\n";
}