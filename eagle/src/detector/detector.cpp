#include "detector.h"
#include "circle_triangle.h"

using namespace eagle;

Detector::Detector(const std::string& config_path, const cv::Mat& background):
    _projection(config_path), _verbose(0),
    _pat(new CircleTriangle(cv::Point2f(0.10, 0.16), cv::Point2f(0.8, 0.5))) {
    cv::FileStorage fs(config_path, cv::FileStorage::READ);
    if (background.empty()) {
        _background = cv::imread(fs["detector"]["background_path"], CV_LOAD_IMAGE_COLOR);
        if (!_background.data) {
            std::cout << "Could not open " << (std::string)fs["detector"]["background_path"] << "!" << std::endl;
        }
    } else {
        _background = background;
    }
    _projection.remap(_background, _background);

    _th_bg_subtraction = (double)fs["detector"]["thresholds"]["bg_subtraction"];
    fs.release();

    // set fixed marker plane at z = 0.77 wrt the world plane for all robots
    // in the future this should be configurable for each robot
    _marker_plane = (cv::Mat_<float>(1, 4) << 0, 0, 1, -0.07);

    // pnp pattern extraction test
    fs = cv::FileStorage(config_path, cv::FileStorage::READ);
    cv::Mat K, D, T;
    fs["camera"]["camera_matrix"] >> K;
    fs["camera"]["distortion_vector"] >> D;
    fs["camera"]["external_transformation"] >> T;
    T.convertTo(T, CV_32F);
    fs.release();
    _extr = new PnpPatternExtractor3(_pat, _projection.camera_matrix(), cv::Mat(0, 0, CV_32F), T);
}

void Detector::search(const cv::Mat& frame, const std::vector<Robot*>& robots, std::vector<Obstacle*>& obstacles) {
    for (uint k = 0; k < robots.size(); k++) {
        robots[k]->reset();
    }
    // undistort frame
    cv::Mat undist;
    _projection.remap(frame, undist);
    cv::Mat mask = get_mask(undist);
    std::vector<std::vector<cv::Point>> contours = get_contours(mask);
    if (!contours.empty()) {
        detect_robots(undist, contours, robots);
        subtract_robots(mask, robots);
        contours = get_contours(mask);
        detect_obstacles(undist, contours, robots, obstacles);
        if (_verbose >= 2) {
            std::cout << "* Detected robots:" << std::endl;
            for (int k=0; k<robots.size(); k++) {
                if (robots[k]->detected()) {
                    std::cout << "\t- " << robots[k]->to_string() << std::endl;
                }
            }
            std::cout << "* Detected obstacles:" << std::endl;
            for (int k=0; k<obstacles.size(); k++) {
                std::cout << "\t- " << obstacles[k]->to_string() << std::endl;
            }
        }
    } else {
        if (_verbose >= 1) {
            std::cout << "No contours detected!" << std::endl;
        }
    }
}

cv::Mat Detector::draw(cv::Mat& frame, const std::vector<Robot*>& robots, const std::vector<Obstacle*>& obstacles) {
    cv::Mat img;
    _projection.remap(frame, img);
    // cv::drawContours(img, _contours, -1, cv::Scalar(255,0,0));

    // coordinate system
    cv::Scalar gray(77, 76, 75);
    cv::Point2f O = _projection.project_to_image(cv::Point3f(0, 0, 0));
    cv::Point2f Ex = _projection.project_to_image(cv::Point3f(1, 0, 0));
    cv::Point2f Ey = _projection.project_to_image(cv::Point3f(0, 1, 0));
    cv::circle(img, O, 5, gray, -2);
    cv::arrowedLine(img, O, Ex, gray, 2); //Ex vector
    cv::arrowedLine(img, O, Ey, gray, 2); //Ey vector
    cv::putText(img, "x", _projection.project_to_image(cv::Point3f(1.1, 0, 0)), cv::FONT_HERSHEY_SIMPLEX, 1, gray, 2);
    cv::putText(img, "y", _projection.project_to_image(cv::Point3f(0, 1.1, 0)), cv::FONT_HERSHEY_SIMPLEX, 1, gray, 2);

    // obstacles
    for (uint i = 0; i < obstacles.size(); i++) {
        obstacles[i]->draw(img, _projection);
        cloud3_t p = obstacles[i]->points();
    }

    // robots
    for (uint i = 0; i < robots.size(); i++) {
        if (robots[i]->detected()) {
            robots[i]->draw(img, _projection);
        }
    }

    return img;
}

void Detector::verbose(int verbose) {
    _verbose = verbose;
}

void Detector::set_background(const cv::Mat& bg) {
    _projection.remap(bg, _background);
}

cv::Mat Detector::get_mask(const cv::Mat& frame) {
    cv::Mat mask;
    // background subtraction
    cv::absdiff(_background, frame, mask);
    cv::cvtColor(mask, mask, CV_RGB2GRAY);
    cv::GaussianBlur(mask, mask, cv::Size(0,0), 2.5, 0);

    cv::threshold(mask, mask, _th_bg_subtraction, 255, cv::THRESH_BINARY);
    // remove speckles by dilation-erosion
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7), cv::Point(3, 3));
    cv::dilate(mask, mask, element);
    cv::erode(mask, mask, element);
    return mask;
}

std::vector<std::vector<cv::Point>> Detector::get_contours(const cv::Mat& mask) {
    // wrapper to findContours
    std::vector<cv::Vec4i> hierarchy;
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    return contours;
}

void Detector::detect_robots(const cv::Mat& frame, const std::vector<std::vector<cv::Point>>& contours, const std::vector<Robot*>& robots) {

    Pattern pat(new CircleTriangle(cv::Point2f(0.10, 0.16), cv::Point2f(0.8, 0.5)));

    std::vector<cv::Point> contour;
    cloud2_t marker_points2(3);
    cloud3_t marker_points3(3);
    cv::Rect roi_rectangle; //region-of-interest rectangle
    cv::Point2f roi_location;
    cv::Mat roi;
    cv::Point2f circle_center;
    float circle_radius;
    int id;
    for (uint i = 0; i < contours.size(); i++) {
        cv::convexHull(contours[i], contour);
        roi_rectangle = cv::boundingRect(contour);
        roi_location = cv::Point2f(roi_rectangle.x, roi_rectangle.y);
        frame(roi_rectangle).copyTo(roi);
        bool search = true;
        while (search) {
            marker_points2 = pat.find(roi, id, false);
            for (uint i = 0; i < marker_points2.size(); i++)
                marker_points2[i] += roi_location;
            marker_points3 = _projection.project_to_plane(marker_points2, _marker_plane);
            // test with pnp extractor
            //cloud3_t pnppoints;
            //pnppoints = _extr->extract(roi, roi_location, id, false);
            for (uint i = 0; i < robots.size(); i++) {
                if (id == robots[i]->code()) {
                    search = true;
                    //robots[i]->update(pnppoints);
                    robots[i]->update(marker_points3);
                }
            }
            if (marker_points3.size() > 0) {
                // subtract markerpoints from roi
                std::vector<cv::Point2f> pnts = _projection.project_to_image(marker_points3);
                std::vector<cv::Point2f> pnts2;
                for (uint l = 0; l < pnts.size(); l++) {
                    cv::Point pnt2 = pnts[l] - roi_location;
                    if (pnt2.x != 0 && pnt2.y != 0) {
                        pnts2.push_back(pnt2);
                    }
                }
                cv::minEnclosingCircle(pnts2, circle_center, circle_radius);
                cv::circle(roi, circle_center, 1.1*circle_radius, cv::Scalar(0, 0, 0), -1);
            } else {
                search = false;
            }
        }
    }
}

void Detector::subtract_robots(cv::Mat& mask, const std::vector<Robot*>& robots) {
    std::vector<std::vector<cv::Point>> robocontours(0);
    for (uint k = 0; k < robots.size(); k++) {
        if (robots[k]->detected()) {
            std::vector<cv::Point2f> vert = _projection.project_to_image(robots[k]->vertices());
            std::vector<cv::Point> vert2(vert.size());
            for (uint l = 0; l < vert.size(); l++) {
                vert2[l] = vert[l];
            }
            robocontours.push_back(vert2);
        }
    }
    // remove robots from mask
    cv::drawContours(mask, robocontours, -1, cv::Scalar(0, 0, 0), -1);
}

void Detector::detect_obstacles(const cv::Mat& frame, const std::vector<std::vector<cv::Point> >& contours, const std::vector<Robot*>& robots, std::vector<Obstacle*>& obstacles) {
    obstacles.clear();
    std::vector<cv::Point> contour;
    std::vector<std::vector<cv::Point>> obstacle_contours(0);
    for (uint i = 0; i < contours.size(); i++) {
        cv::convexHull(contours[i], contour);
        //if (cv::contourArea(contour) > _min_obstacle_area*pow(_meter2pixel, 2)) {
        obstacle_contours.push_back(contour);
        //}
    }
    filter_obstacles(obstacle_contours, robots, obstacles);
}

void Detector::filter_obstacles(const std::vector<std::vector<cv::Point>>& contours, const std::vector<Robot*>& robots, std::vector<Obstacle*>& obstacles) {
    cv::RotatedRect rect;
    cv::Point2f center;
    float radius;
    bool add = true;
    // Compute robot vertices in 2d
    std::vector<cloud2_t> robot_points2(robots.size());
    for (uint i = 0; i < robots.size(); i++) {
        if (robots[i]->detected()) {
            robot_points2[i] = dropz(robots[i]->vertices()); // project to ground
        }
    }
    // Compute contour vertices in 2d (and 3d.)
    cv::Mat ground_plane = (cv::Mat_<float>(1, 4) << 0, 0, 1, 0);
    std::vector<cloud2_t> obstacle_points2 = dropz(_projection.project_to_plane(contours, ground_plane)); // project pixels to ground
    for (uint i = 0; i < obstacle_points2.size(); i++) {
        if (contourArea(obstacle_points2[i]) > 0.01) {
            rect = cv::minAreaRect(obstacle_points2[i]);
            add = true;

            for (uint j = 0; (j < robots.size()) && add; j++) {
                if (robots[j]->detected()) {
                    add = cv::pointPolygonTest(robot_points2[j], rect.center, false) < 0;
                }
            }
            for (uint j = 0; (j < contours.size()) && add; j++) {
                if (i != j) {
                    add = cv::pointPolygonTest(obstacle_points2[j], rect.center, false) < 0;
                }
            }
            if (add) {
                cv::minEnclosingCircle(obstacle_points2[i], center, radius);
                if (rect.size.area() < (M_PI * radius * radius)) {
                    obstacles.push_back(new RectangleObstacle(rect));
                } else {
                    obstacles.push_back(new CircleObstacle(center, radius));
                }
            }
        }
    }
    // sort obstacles according to area
    // sort_obstacles(obstacles);
}

void Detector::sort_obstacles(std::vector<Obstacle*>& obstacles) {
   int j;
   Obstacle* obstacle;
   // insertion sort
   for (int i=1; i<obstacles.size(); i++) {
       j = i;
       while (j > 0 && obstacles[j-1]->area() < obstacles[j]->area()) {
           // swap obstacles
           obstacle = obstacles[j];
           obstacles[j] = obstacles[j-1];
           obstacles[j-1] = obstacle;
       }
       j--;
   }
}

