#include "detector.h"

Detector::Detector(const std::string& param_file, const cv::Mat& background) :
    _background(background) {
    read_parameters(param_file);
}

Detector::Detector(const std::string& param_file, const std::string& background_path) {
    cv::Mat background = cv::imread(background_path, CV_LOAD_IMAGE_COLOR);
    if (!background.data) {
        std::cout << "Could not open " << background_path << "!" << std::endl;
    }
    Detector(param_file, background);
}

void Detector::read_parameters(const std::string& param_file) {
    cv::FileStorage params(param_file, cv::FileStorage::READ);
    _meter2pixel = (double)params["pixelspermeter"];
    _pixel2meter = 1./_meter2pixel;
    _min_robot_area = (double)params["min_robot_area"];
    _min_obstacle_area = (double)params["min_obstacle_area"];
    _triangle_ratio = (double)params["markers"]["triangle_ratio"];
    _qr_posx = (double)params["markers"]["qr_posx"];
    _qr_posy = (double)params["markers"]["qr_posy"];
    _qr_sizex = (double)params["markers"]["qr_sizex"];
    _qr_sizey = (double)params["markers"]["qr_sizey"];
    _qr_nbitx = (double)params["markers"]["qr_nbitx"];
    _qr_nbity = (double)params["markers"]["qr_nbity"];
    _th_triangle_ratio = (double)params["thresholds"]["triangle_ratio"];
    _th_top_marker = (double)params["thresholds"]["top_marker"];
    _th_bg_subtraction = (int)params["thresholds"]["bg_subtraction"];
    init_blob(params);
}

void Detector::init_blob(const cv::FileStorage& params) {
    cv::SimpleBlobDetector::Params blob_par;
    blob_par.minThreshold = (int)params["blob"]["minThreshold"];
    blob_par.maxThreshold = (int)params["blob"]["maxThreshold"];
    blob_par.filterByArea = (int)params["blob"]["filterByArea"];
    blob_par.minArea = (int)params["blob"]["minArea"];
    blob_par.filterByCircularity = (int)params["blob"]["filterByCircularity"];
    blob_par.minCircularity = (double)params["blob"]["minCircularity"];
    _blob_detector = cv::SimpleBlobDetector(blob_par);
}

void Detector::init_transformations(const cv::Mat& frame) {
    int height = frame.size().height;
    _cam2world_tf = cv::Matx23f(_pixel2meter, 0, 0, 0, -_pixel2meter, _pixel2meter*height);
    _world2cam_tf = cv::Matx23f(_meter2pixel, 0, 0, 0, -_meter2pixel, height);
}

void Detector::search(const cv::Mat& frame, const std::vector<Robot*>& robots, std::vector<Obstacle*>& obstacles) {
    init_transformations(frame);
    std::vector<std::vector<cv::Point> > contours;
    if (!subtract_background(frame, contours)) {
        return; // no contours were found
    }
    detect_robots(frame, contours, robots);
    if (!subtract_robots(contours, robots)) {
        return; // no obstacles were found
    }
    detect_obstacles(frame, contours, robots, obstacles);
}

void Detector::draw(cv::Mat& frame, const std::vector<Robot*>& robots, const std::vector<Obstacle*>& obstacles) {
    cv::Size size = frame.size();
    // coordinate system
    cv::Scalar gray(77, 76, 75);
    cv::circle(frame, cv::Point2i(0, size.height), 5, gray, -2);
    cv::line(frame, cv::Point2i(0, size.height), cv::Point2i(20, size.height), gray, 2);
    cv::line(frame, cv::Point2i(0, size.height), cv::Point2i(0, size.height-20), gray, 2);
    for (int i=0; i<(size.height*_pixel2meter); i++) {
        cv::line(frame, cv::Point2i(0, size.height-(i+1)*_meter2pixel), cv::Point2i(5, size.height-(i+1)*_meter2pixel), gray, 2);
    }
    for (int i=0; i<(size.width*_pixel2meter); i++) {
        cv::line(frame, cv::Point2i((i+1)*_meter2pixel, size.height), cv::Point2i((i+1)*_meter2pixel, size.height-5), gray, 2);
    }
    // obstacles
    for (uint i=0; i<obstacles.size(); i++) {
        obstacles[i]->draw(frame, _world2cam_tf);
    }
    // robots
    for (uint i=0; i<robots.size(); i++) {
        robots[i]->draw(frame, _world2cam_tf);
    }
}

bool Detector::subtract_background(const cv::Mat& frame, std::vector<std::vector<cv::Point>>& contours) {
    cv::Mat mask_copy;
    cv::absdiff(_background, frame, _cont_mask);
    cv::cvtColor(_cont_mask, _cont_mask, CV_RGB2GRAY);
    cv::threshold(_cont_mask, _cont_mask, _th_bg_subtraction, 255, cv::THRESH_BINARY);
    _cont_mask.copyTo(mask_copy);
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(mask_copy, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    if (hierarchy.empty()) {
      return false;
    }
    return true;
}

void Detector::detect_robots(const cv::Mat& frame, const std::vector<std::vector<cv::Point>>& contours, const std::vector<Robot*>& robots) {
    std::vector<cv::Point> contour;
    cv::Rect roi_rectangle; //region-of-interest rectangle
    cv::Point2f roi_location;
    cv::Mat roi;
    for (uint i=0; i<contours.size(); i++) {
        cv::convexHull(contours[i], contour);
        if (cv::contourArea(contour) > _min_robot_area*pow(_meter2pixel, 2)) {
            roi_rectangle = cv::boundingRect(contour);
            roi_location = cv::Point2f(roi_rectangle.x, roi_rectangle.y);
            frame(roi_rectangle).copyTo(roi);
            find_robots(roi, roi_location, robots);
        }
    }
}

void Detector::find_robots(cv::Mat& roi, const cv::Point2f& roi_location, const std::vector<Robot*>& robots) {
    // detect blobs
    cv::cvtColor(roi, roi, CV_RGB2GRAY);
    std::vector<cv::KeyPoint> blobs;
    cv::resize(roi, roi, cv::Size(), 2, 2);
    _blob_detector.detect(roi, blobs);
    if (blobs.size() < 3) {
        return;
    }
    // get all possible combinations of 3 points
    std::vector<cv::Point2f> points(3);
    std::vector<bool> selector(blobs.size());
    std::fill(selector.begin(), selector.begin() + 3, true);
    do {
        uint k = 0;
        for (uint i=0; i < blobs.size(); i++) {
            if (selector[i]) {
                points[k] = blobs[i].pt;
                k++;
            }
        }
        // check if combination decodes to a valid robot
        decode_robot(roi, roi_location, points, robots);
    } while (std::prev_permutation(selector.begin(), selector.end()));
}

void Detector::decode_robot(const cv::Mat& roi, const cv::Point2f& roi_location, const std::vector<cv::Point2f>& points, const std::vector<Robot*>& robots) {
    std::vector<cv::Point2f> markers(3);
    if (!get_markers(points, markers)) {
        return;
    }
    // decode QR
    cv::Point2f zero = 0.5*(markers[0]+markers[1]);
    cv::Point2f step_x = (1./(_qr_nbitx))*_qr_sizex*(markers[2]-zero);
    cv::Point2f step_y = -(1./(_qr_nbity))*_qr_sizey*(markers[0]-zero);
    cv::Point2f upperleft_qr = zero + (_qr_posx - 0.5*_qr_sizex)*(markers[2]-zero) + (_qr_posy + 0.5*_qr_sizey)*(markers[0]-zero);
    upperleft_qr += 0.5*step_x + 0.5*step_y;
    cv::Mat roi_mask;
    roi.copyTo(roi_mask);
    // cv::GaussianBlur(roi_mask, roi_mask, cv::Size(5, 5), 1, 1);
    cv::threshold(roi_mask, roi_mask, 100, 255, cv::THRESH_BINARY);
    cv::Scalar color;
    cv::Point2f point;
    uint code = 0;
    uint bit_selector = 1;
    for (int k=0; k<_qr_nbitx; k++) {
        for (int l=0; l<_qr_nbity; l++) {
            point = upperleft_qr + k*step_x + l*step_y;
            color = roi_mask.at<uchar>(point);
            if (color.val[0] == 0) {
                code |= (bit_selector << (k+2*l));
            }
        }
    }
    // assign to robot
    for (uint i=0; i<robots.size(); i++) {
        if (code == robots[i]->code()) {
            std::vector<cv::Point2f> markers_glob(3);
            for (int i=0; i<3; i++) {
                markers_glob[i] = markers[i] + roi_location;
            }
            robots[i]->update(cam2worldframe(markers_glob));
            return;
        }
    }
}

bool Detector::get_markers(const std::vector<cv::Point2f>& points, std::vector<cv::Point2f>& markers) {
    // find top and bottom markers
    std::vector<double> dist(3);
    dist[0] = cv::norm(points[0] - points[1]);
    dist[1] = cv::norm(points[1] - points[2]);
    dist[2] = cv::norm(points[2] - points[0]);
    int top_ind;
    if (fabs(dist[0]-dist[1])/dist[1] < _th_top_marker) {
        top_ind = 1;
    } else if (fabs(dist[1]-dist[2])/dist[2] < _th_top_marker) {
        top_ind = 2;
    } else if (fabs(dist[0]-dist[2])/dist[2] < _th_top_marker) {
        top_ind = 0;
    } else {
        return false;
    }
    int btm_ind[2];
    int k=0;
    for (int l=0; l<3; l++) {
        if (l != top_ind) {
            btm_ind[k] = l;
            k++;
        }
    }
    // check if valid triangle
    cv::Point2f midbottom = 0.5*(points[btm_ind[0]]+points[btm_ind[1]]);
    double width = cv::norm(points[btm_ind[0]] - points[btm_ind[1]]);
    double height = cv::norm(points[top_ind] - midbottom);
    if (fabs(height/width - _triangle_ratio)/_triangle_ratio > _th_triangle_ratio) {
        return false;
    }
    // order markers: left - right - top
    markers.resize(3);
    double cp = (points[btm_ind[1]]-points[btm_ind[0]]).cross(points[top_ind]-points[btm_ind[0]]);
    if (cp >= 0) {
        markers[0] = points[btm_ind[0]];
        markers[1] = points[btm_ind[1]];
    } else {
        markers[0] = points[btm_ind[1]];
        markers[1] = points[btm_ind[0]];
    }
    markers[2] = points[top_ind];
    return true;
}

bool Detector::subtract_robots(std::vector<std::vector<cv::Point>>& contours, const std::vector<Robot*>& robots) {
    std::vector<std::vector<cv::Point2f>> robocontours(0);
    for (uint k=0; k<robots.size(); k++) {
        if (robots[k]->detected()) {
            robocontours.push_back(world2camframe(robots[k]->vertices()));
        }
    }
    // remove robots from mask
    cv::drawContours(_cont_mask, robocontours, 0, cv::Scalar(0,0,0), -1);
    // find new contours
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(_cont_mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    if (hierarchy.empty()) {
        return false;
    }
    return true;
}

void Detector::detect_obstacles(const cv::Mat& frame, const std::vector<std::vector<cv::Point> >& contours, const std::vector<Robot*>& robots, std::vector<Obstacle*>& obstacles) {
    obstacles.clear();
    std::vector<cv::Point> contour;
    std::vector<std::vector<cv::Point>> obstacle_contours(0);
    for (uint i=0; i<contours.size(); i++) {
        cv::convexHull(contours[i], contour);
        if (cv::contourArea(contour) > _min_obstacle_area*pow(_meter2pixel, 2)) {
            obstacle_contours.push_back(contour);
        }
    }
    filter_obstacles(obstacle_contours, robots, obstacles);
}

void Detector::filter_obstacles(const std::vector<std::vector<cv::Point>>& contours, const std::vector<Robot*>& robots, std::vector<Obstacle*>& obstacles) {
    cv::RotatedRect rect;
    cv::Point2f center;
    float radius;
    bool add = true;
    for (uint i=0; i<contours.size(); i++) {
        rect = cv::minAreaRect(contours[i]);
        for (uint j=0; j<robots.size(); j++) {
            if (cv::pointPolygonTest(world2camframe(robots[j]->vertices()), rect.center, false) > 0) {
                add = false; // obstacle is within a robot
            }
        }
        for (uint j=0; j<contours.size(); j++) {
            if ((i!=j) && cv::pointPolygonTest(contours[j], rect.center, false) > 0) {
                add = false; // obstacle is within another obstacle
            }
        }
        if (add) {
            cv::minEnclosingCircle(contours[i], center, radius);
            if (rect.size.width*rect.size.height < M_PI*pow(radius, 2)) {
                obstacles.push_back(new Rectangle(cam2worldframe(rect.center), -rect.angle, rect.size.width*_pixel2meter, rect.size.height*_pixel2meter));
            } else {
                obstacles.push_back(new Circle(cam2worldframe(center), radius*_pixel2meter));
            }
        }
    }
    sort_obstacles(obstacles);
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

cv::Point2f Detector::cam2worldframe(const cv::Point2f& point) {
    return cam2worldframe(std::vector<cv::Point2f>{point})[0];
}

std::vector<cv::Point2f> Detector::cam2worldframe(const std::vector<cv::Point2f>& points) {
    std::vector<cv::Point2f> ret;
    cv::transform(points, ret, _cam2world_tf);
    return ret;
}

cv::Point2f Detector::world2camframe(const cv::Point2f& point) {
    return world2camframe(std::vector<cv::Point2f>{point})[0];
}

std::vector<cv::Point2f> Detector::world2camframe(const std::vector<cv::Point2f>& points) {
    std::vector<cv::Point2f> ret;
    cv::transform(points, ret, _world2cam_tf);
    return ret;
}
