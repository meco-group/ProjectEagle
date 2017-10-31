#include "detector.h"
#include <circle_triangle.h>

using namespace eagle;

Detector::Detector(const std::string& config_path, const cv::Mat& background):
    _projection(config_path),
    _pat(new CircleTriangle(cv::Point2f(0.10,0.16), cv::Point2f(0.8,0.5)))
{
    cv::FileStorage fs(config_path, cv::FileStorage::READ);
    if(background.empty()) {
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

    // set fixed ground/marker plane at z = 0.1 wrt the world plane 
    _ground = (cv::Mat_<float>(1,4) << 0,0,1,-0.07); 
    

    // pnp pattern extraction test
    fs = cv::FileStorage(config_path, cv::FileStorage::READ);
    cv::Mat K, D, T;
    fs["camera"]["camera_matrix"] >> K;
    fs["camera"]["distortion_vector"] >> D;
    fs["camera"]["external_transformation"] >> T;
    T.convertTo(T,CV_32F);
    fs.release();
    _extr = new PnpPatternExtractor3(_pat, _projection.camera_matrix(), cv::Mat(0,0,CV_32F), T);
}

void Detector::search(const cv::Mat& frame, const std::vector<Robot*>& robots, std::vector<Obstacle*>& obstacles) {
    for (uint k=0; k<robots.size(); k++) {
        robots[k]->reset();
    }

    // undistort frame
    cv::Mat undist; 
    _projection.remap(frame,undist);
    cv::Mat mask = get_mask(undist);
    std::vector<std::vector<cv::Point>> contours = get_contours(mask);
    if (!contours.empty()) {
        detect_robots(undist, contours, robots);
        subtract_robots(mask, robots);
        contours = get_contours(mask);
        detect_obstacles(frame, contours, robots, obstacles);
    } else {
        std::cout << "No ROIs detected" << std::endl;
    }
}

cv::Mat Detector::draw(cv::Mat& frame, const std::vector<Robot*>& robots, const std::vector<Obstacle*>& obstacles) {
    cv::Mat img;
    _projection.remap(frame, img);

    // coordinate system
    cv::Scalar gray(77, 76, 75);
    cv::Point2f O = _projection.project_to_image(cv::Point3f(0,0,0));
    cv::Point2f Ex = _projection.project_to_image(cv::Point3f(1,0,0));
    cv::Point2f Ey = _projection.project_to_image(cv::Point3f(0,1,0));
    cv::circle(img, O, 5, gray, -2);
    cv::arrowedLine(img, O, Ex, gray, 2); //Ex vector
    cv::arrowedLine(img, O, Ey, gray, 2); //Ey vector
    cv::putText(img, "x", _projection.project_to_image(cv::Point3f(1.1,0,0)), cv::FONT_HERSHEY_SIMPLEX, 1, gray, 2);
    cv::putText(img, "y", _projection.project_to_image(cv::Point3f(0,1.1,0)), cv::FONT_HERSHEY_SIMPLEX, 1, gray, 2);

    // obstacles
    for (uint i=0; i<obstacles.size(); i++) {
        obstacles[i]->draw(img, _projection);
        cloud3_t p = obstacles[i]->points();
        for (uint j=0; j<p.size(); j++) {
//            std::cout << p[j] << std::endl;
//            std::cout << obstacles[i]->area() << std::endl;
        }
    }

    // robots
    for (uint i=0; i<robots.size(); i++) {
        if (robots[i]->detected()) {
            robots[i]->draw(img, _projection);
        }
    }

    return img;
}
cv::Mat Detector::get_mask(const cv::Mat& frame) {
    cv::Mat mask;
    // background subtraction
    cv::absdiff(_background, frame, mask);
    cv::cvtColor(mask, mask, CV_RGB2GRAY);
    cv::threshold(mask, mask, _th_bg_subtraction, 255, cv::THRESH_BINARY);
    // remove speckles by dilation-erosion
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7,7), cv::Point(3,3));
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
    
    Pattern pat(new CircleTriangle(cv::Point2f(0.10,0.16), cv::Point2f(0.8,0.5)));

    std::vector<cv::Point> contour;
    cloud2_t marker_points2(3);
    cloud3_t marker_points3(3);
    cv::Rect roi_rectangle; //region-of-interest rectangle
    cv::Point2f roi_location;
    cv::Mat roi;
    int id;
    for (uint i=0; i<contours.size(); i++) {
        cv::convexHull(contours[i], contour);
        roi_rectangle = cv::boundingRect(contour);
        roi_location = cv::Point2f(roi_rectangle.x, roi_rectangle.y);
        frame(roi_rectangle).copyTo(roi);
        
        marker_points2 = pat.find(roi, id, false);
        for (uint i=0; i<marker_points2.size(); i++)
            marker_points2[i] += roi_location;

        marker_points3 = _projection.project_to_plane(marker_points2, _ground);

        // test with pnp extractor
        cloud3_t pnppoints;
        pnppoints = _extr->extract(roi, roi_location, id, false);
        
        for (uint i=0; i<robots.size(); i++) {
            if (id == robots[i]->code()) {
                robots[i]->update(pnppoints);// marker_points3);
                std::cout << "Marker [" << id << "] at " << robots[i]->translation() << robots[i]->rotation() << std::endl;
            }
        }
    }
}

void Detector::subtract_robots(cv::Mat& mask, const std::vector<Robot*>& robots) {
    std::vector<std::vector<cv::Point>> robocontours(0);
    for (uint k=0; k<robots.size(); k++) {
        if (robots[k]->detected()) {
            std::vector<cv::Point2f> vert = _projection.project_to_image(robots[k]->vertices());
            std::vector<cv::Point> vert2(vert.size());
            for (uint l=0; l<vert.size(); l++) {
                vert2[l] = vert[l];
            }
            robocontours.push_back(vert2);
        }
    }
    // remove robots from mask
    cv::drawContours(mask, robocontours, -1, cv::Scalar(0,0,0), -1);
}

void Detector::detect_obstacles(const cv::Mat& frame, const std::vector<std::vector<cv::Point> >& contours, const std::vector<Robot*>& robots, std::vector<Obstacle*>& obstacles) {
    obstacles.clear();
    std::vector<cv::Point> contour;
    std::vector<std::vector<cv::Point>> obstacle_contours(0);
    for (uint i=0; i<contours.size(); i++) {
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

    // Construct transform - ground refers to local, projection to global so times inv!
    cv::Mat T_inplane = compute_inplane_transform(transform_plane(_projection.get_transform().inv(), _ground));

    // Compute robot vertices in 2d
    std::vector<cloud2_t> robot_points2(robots.size());
    for (uint i=0; i<robots.size(); i++) {
        if (robots[i]->detected()) {
            robot_points2[i] = transform2(T_inplane, robots[i]->vertices());
        }
    }

    // Compute contour vertices in 2d (and 3d.)
    std::vector<cloud2_t> obstacle_points2 = transform2(T_inplane, _projection.project_to_plane(contours, _ground));
        
    //cloud3_t obstacle_points3 = transform(T, _projection.project_to_plane(contours[i], _ground));

    for (uint i=0; i<obstacle_points2.size(); i++) {
        if (contourArea(obstacle_points2[i]) > 0.01) {
            rect = cv::minAreaRect(obstacle_points2[i]);
            add = true;
    
            for (uint j=0; (j<robots.size()) && add; j++) {
                if (robots[j]->detected()) {
                    add = cv::pointPolygonTest(robot_points2[j], rect.center, false) < 0;
                }
            }
            for (uint j=0; (j<contours.size()) && add; j++) {
                if (i!=j) {
                    add = cv::pointPolygonTest(obstacle_points2[j], rect.center, false) < 0;
                }
            }
    
            if (add) {
                cv::minEnclosingCircle(obstacle_points2[i], center, radius);
                if (rect.size.area() < (M_PI*radius*radius)) {
                    obstacles.push_back(new RectangleObstacle(rect, cv::Mat(T_inplane.inv())));
                } else {
                    obstacles.push_back(new CircleObstacle(center, radius, cv::Mat(T_inplane.inv())));
                }
            }
        }
    }
    
    // sort obstacles according to area
    
//    sort_obstacles(obstacles);
}
//
//void Detector::sort_obstacles(std::vector<Obstacle*>& obstacles) {
//    int j;
//    Obstacle* obstacle;
//    // insertion sort
//    for (int i=1; i<obstacles.size(); i++) {
//        j = i;
//        while (j > 0 && obstacles[j-1]->area() < obstacles[j]->area()) {
//            // swap obstacles
//            obstacle = obstacles[j];
//            obstacles[j] = obstacles[j-1];
//            obstacles[j-1] = obstacle;
//        }
//        j--;
//    }
//}
//
//cv::Point2f Detector::cam2worldframe(const cv::Point2f& point) {
//    return cam2worldframe(std::vector<cv::Point2f>{point})[0];
//}
//
//std::vector<cv::Point2f> Detector::cam2worldframe(const std::vector<cv::Point2f>& points) {
//    std::vector<cv::Point2f> ret;
//    cv::transform(points, ret, _cam2world_tf);
//    return ret;
//}
//
//cv::Point2f Detector::world2camframe(const cv::Point2f& point) {
//    return world2camframe(std::vector<cv::Point2f>{point})[0];
//}
//
//std::vector<cv::Point2f> Detector::world2camframe(const std::vector<cv::Point2f>& points) {
//    std::vector<cv::Point2f> ret;
//    cv::transform(points, ret, _world2cam_tf);
//    return ret;
//}
