#include <circle_triangle.h>
#include <opencv2/imgproc/imgproc.hpp>

using namespace eagle;

CircleTriangle::CircleTriangle(const cv::String& config)
{
    cv::FileStorage fs(config, cv::FileStorage::READ);
    double min_obstacle_area = (double)fs["detector"]["min_obstacle_area"];
    double triangle_ratio = (double)fs["detector"]["markers"]["triangle_ratio"];
    double qr_posx = (double)fs["detector"]["markers"]["qr_posx"];
    double qr_posy = (double)fs["detector"]["markers"]["qr_posy"];
    double qr_sizex = (double)fs["detector"]["markers"]["qr_sizex"];
    double qr_sizey = (double)fs["detector"]["markers"]["qr_sizey"];
    double qr_nbitx = (int)fs["detector"]["markers"]["qr_nbitx"];
    double qr_nbity = (int)fs["detector"]["markers"]["qr_nbity"];
    _th_triangle_ratio = (double)fs["detector"]["thresholds"]["triangle_ratio"];
    _th_top_marker = (double)fs["detector"]["thresholds"]["top_marker"];
    fs.release();
}

CircleTriangle::CircleTriangle(const cv::Point2f& dimension, const cv::Point2f& qr_size, const cv::Point2f& qr_pos, const cv::Point2i& qr_n) :
    _dimension(dimension), _qr_size(qr_size), _qr_pos(qr_pos), _qr_n(qr_n)
{

}

std::vector<cv::Point2f> CircleTriangle::find(cv::Mat& img) const 
{
    int t;
    return find(img, t, false);
}

std::vector<cv::Point2f> CircleTriangle::find(cv::Mat& img, int& id, bool draw) const
{
    bool found = false;
    std::vector<cv::Point2f> points(7);
    int scale = 2; //adaptive scaling possible?

    // blob detection
    cv::Mat roi;
    cv::cvtColor(img, roi, CV_RGB2GRAY);
    cv::resize(roi, roi, cv::Size(), scale, scale); //is this necessary? => yes! otherwise, blob detection is unstable and code is hard to decode
    std::vector<cv::KeyPoint> blobs;

    //initialize blob detector
    cv::SimpleBlobDetector::Params blob_par;
    blob_par.minThreshold = 10;
    blob_par.maxThreshold = 200;
    blob_par.filterByArea = 1;
    blob_par.minArea = 80;
    blob_par.filterByCircularity = 1;
    blob_par.minCircularity = 0.85;
    cv::Ptr<cv::SimpleBlobDetector> blob_detector = cv::SimpleBlobDetector::create(blob_par);
    blob_detector->detect(roi, blobs);
    
    // do some
//    cv::Mat roic;
//    cv::adaptiveThreshold(roi,roic,255,cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 5, 3);
//    cv::Mat roic2; 
//    cv::cornerHarris(roic,roic2,11,3,0.04);
//    for (uint k=0; k<blobs.size(); k++){
//        cv::circle(roi, blobs[k].pt, 5, cv::Scalar(255,0,0), -2);
//    }
//    cv::imshow("ROI",roi);
////    cv::imshow("ROIC",roic2);
//    cv::waitKey(0);

    // get all possible combinations of 3 points: are these really all combinations?? checked and valid!!
    if( blobs.size() >= 3) {
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
            if (check(points)) {
                std::cout << "Standard detection method found marker" << std::endl;
                check(roi, points);
                id = decode(roi, points);
                for (uint i=0; i<points.size(); i++) {
                    points[i] /= scale;
                }
                found = true;
            }
        } while ((!found) && std::prev_permutation(selector.begin(), selector.end()));
    }

    if (!found) {
        points.clear();
        id = -1;
    }

    return points;
}

std::vector<cv::Point3f> CircleTriangle::reference() const 
{
    std::vector<cv::Point3f> points(7);
    points[0] = cv::Point3f(0,_dimension.y*0.5,0);
    points[1] = cv::Point3f(0,-_dimension.y*0.5,0);
    points[2] = cv::Point3f(_dimension.x,0,0);

    cv::Point3f offset(_dimension.x*_qr_pos.x, _dimension.y*_qr_pos.y, 0);
    points[3] = 0.5*cv::Point3f(-_dimension.x*_qr_size.x, _dimension.y*_qr_size.y, 0) + offset;
    points[4] = 0.5*cv::Point3f(-_dimension.x*_qr_size.x, -_dimension.y*_qr_size.y, 0) + offset;
    points[5] = 0.5*cv::Point3f(_dimension.x*_qr_size.x, -_dimension.y*_qr_size.y, 0) + offset;
    points[6] = 0.5*cv::Point3f(_dimension.x*_qr_size.x, _dimension.y*_qr_size.y, 0) + offset;

    return points;
}

bool CircleTriangle::check(std::vector<cv::Point2f>& points) const
{
    // find top and bottom markers
    std::vector<double> dist(3);
    dist[0] = cv::norm(points[0] - points[1]);
    dist[1] = cv::norm(points[1] - points[2]);
    dist[2] = cv::norm(points[2] - points[0]);
    int top_ind;
    if (std::fabs(dist[0]-dist[1])/dist[1] < _th_top_marker) {
        top_ind = 1;
    } else if (std::fabs(dist[1]-dist[2])/dist[2] < _th_top_marker) {
        top_ind = 2;
    } else if (std::fabs(dist[0]-dist[2])/dist[2] < _th_top_marker) {
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
    
    // order markers: left - right - top
    std::vector<cv::Point2f> markers(3);
    double cp = (points[btm_ind[1]]-points[btm_ind[0]]).cross(points[top_ind]-points[btm_ind[0]]);
    if (cp <= 0) {
        markers[0] = points[btm_ind[0]];
        markers[1] = points[btm_ind[1]];
    } else {
        markers[0] = points[btm_ind[1]];
        markers[1] = points[btm_ind[0]];
    }
    markers[2] = points[top_ind];

    for (uint k=0; k<3; k++) {
        points[k] = markers[k];
    }

    return (std::fabs(height/width - ratio())/ratio() < _th_triangle_ratio);
}

bool CircleTriangle::check(const cv::Mat& img, std::vector<cv::Point2f>& points) const
{
    bool found;
    
    // determine scale
    int z = std::round(std::max(1.0, 300.0/cv::arcLength(points,true)));
    cv::Mat res;
    resize(img,res,cv::Size(),z,z,cv::INTER_CUBIC);

    //Threshold image to get contours
    cv::Mat bin;
    cv::threshold(res, bin, 150, 255, cv::THRESH_BINARY_INV);

    //Select region of interest for contours
    cv::Point2f center = (points[0] + points[1] + points[2])/3;
    double radius = std::max(cv::norm(center-points[0]), cv::norm(center-points[1]));
    radius = std::max(radius*1.4, cv::norm(center-points[2]));
    
    cv::Mat mask(bin.size[0], bin.size[1], CV_8U, cv::Scalar(0));
    cv::circle(mask, center*z, radius*z, 255, -2);
    cv::bitwise_and(mask, bin, bin);

    //Find contour with maximum area
    std::vector<cv::Vec4i> hierarchy;
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(bin, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (contours.empty()) {
        std::cout << "Check failed because no contours were found" << std::endl;
        return false;
    }

    int max_l = 0;
    double area = cv::contourArea(contours[0]);
    double t_area;
    std::vector<cv::Point2f> points_refined(3);
    for (uint l=0; l<contours.size(); l++) {
        t_area = cv::contourArea(contours[l]);
        if( t_area > area ) {
            max_l = l;
            area = t_area;
        }
    }
    std::vector<cv::Point2f> corners;
    cv::approxPolyDP(contours[max_l],corners,5,true);

    // Rescale corners
    for (uint k=0; k<corners.size(); k++){
        cv::circle(res, corners[k], 4, cv::Scalar(0,0,255), 1);
        corners[k] /= z;
    }
    for (uint k=0; k<points.size(); k++){
        cv::circle(res, points[k]*z, 4, cv::Scalar(0,255,0), 1);
    }

    if (corners.size() !=4) {
        std::cout << "Biggest contour rejected because the number of corners > 4" << std::cout;
        return false;
    }
        
    //reorder points
    cv::Point2f square_center = (corners[0] + corners[1] + corners[2] + corners[3])*0.25;
    std::vector<cv::Point2f> mid = {(points[1] + points[2])*0.5, (points[2] + points[0])*0.5, (points[0] + points[1])*0.5};

    double dist = cv::norm(square_center - mid[0]);
    int top = 0;
    std::vector<int> bot;
    double t;
    for (uint l=1; l<3; l++) {
        t = cv::norm(square_center - mid[l]);
        if (dist > t) {
            bot.push_back(top);
            dist = t;
            top = l;
        } else {
            bot.push_back(l);
        }
    }

    //std::cout << "Distance to center found: " << dist << std::endl << "top index: " << top << std::endl; 

    double cp = (points[bot[1]]-points[bot[0]]).cross(points[top]-points[bot[0]]);
    std::vector<cv::Point2f> points_copy = points;
    if (cp <= 0) {
        points[0] = points_copy[bot[0]];
        points[1] = points_copy[bot[1]];
    } else {
        points[0] = points_copy[bot[1]];
        points[1] = points_copy[bot[0]];
    }
    points[2] = points_copy[top];

    //reorder marker corners
    if (cv::contourArea(corners,true) > 0) {
        std::reverse(corners.begin(), corners.end());
    }

    cv::Point2f vx = points[1] - points[0];
    cv::Point2f vy = points[2] - 0.5*(points[0] + points[1]);
    cv::Point2f tp;
    double s;
    std::vector<int> start_idx;
    for (uint l=0; l<4; l++) {
        tp = corners[l] - square_center;
        if ((tp.dot(vx)<0) && (tp.dot(vy)<0)) {
            start_idx.push_back(l);
        }

        //rescale by half a pixel
        s = cv::norm(tp);
        corners[l] = square_center + tp*((s+0.5)/s);
    }

    if (start_idx.size() != 1) {
        std::cout << "Multiple corners identified as corner 1" << std::endl;
        return false;
    }
    std::rotate(corners.begin(), corners.begin()+start_idx[0], corners.end());
    points.erase(points.begin()+3,points.end());
    points.insert(points.end(), corners.begin(), corners.end());

    for (uint l=0; l<points.size(); l++) {
        cv::putText(res, cv::format("%i",l), points[l]*z, cv::FONT_HERSHEY_SCRIPT_SIMPLEX, 1, cv::Scalar(127,0,0));
    }

//    cv::imshow("numbers",res);
//    cv::waitKey(0);

    return true;
}

int CircleTriangle::decode(cv::Mat& img, const std::vector<cv::Point2f>& points) const
{
    cv::Mat roi;
    img.copyTo(roi);

    // decode QR
    cv::Point2f zero = 0.5*(points[0]+points[1]);
    cv::Point2f step_x = (_qr_size.x/_qr_n.x)*(points[2]-zero);
    cv::Point2f step_y = -(_qr_size.y/_qr_n.y)*(points[0]-zero);
    cv::Point2f upperleft_qr = zero + (_qr_pos.x - 0.5*_qr_size.x)*(points[2]-zero) + (_qr_pos.y + 0.5*_qr_size.y)*(points[0]-zero);
    upperleft_qr += 0.5*step_x + 0.5*step_y;
    cv::Mat roi_mask;
    roi.copyTo(roi_mask);
    // cv::GaussianBlur(roi_mask, roi_mask, cv::Size(5, 5), 1, 1);
    cv::threshold(roi_mask, roi_mask, 100, 255, cv::THRESH_BINARY);
    cv::Scalar color;
    cv::Point2f point;
    uint code = 0;
    uint bit_selector = 1;
    for (int k=0; k<_qr_n.x; k++) {
        for (int l=0; l<_qr_n.y; l++) {
            point = upperleft_qr + k*step_x + l*step_y;
            color = roi_mask.at<uchar>(point);
            if (color.val[0] == 0) {
                code |= (bit_selector << (k+2*l));
            }
        }
    }

    return code;
}
