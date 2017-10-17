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
    std::vector<cv::Point2f> points(3);

    // blob detection
    cv::Mat roi;
    cv::cvtColor(img, roi, CV_RGB2GRAY);
    std::vector<cv::KeyPoint> blobs;
    //cv::resize(roi, roi, cv::Size(), 2, 2); //is this necessary?

    //initialize blob detector
    cv::SimpleBlobDetector::Params blob_par;
    blob_par.minThreshold = 10;
    blob_par.maxThreshold = 200;
    blob_par.filterByArea = 1;
    blob_par.minArea = 20;
    blob_par.filterByCircularity = 1;
    blob_par.minCircularity = 0.85;
    cv::Ptr<cv::SimpleBlobDetector> blob_detector = cv::SimpleBlobDetector::create(blob_par);
    //cv::resize(roi, roi, cv::Size(), 2, 2);
    blob_detector->detect(roi, blobs);
    
    // do some
//    cv::Mat roic;
//    cv::adaptiveThreshold(roi,roic,255,cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 5, 3);
//    cv::Mat roic2; 
//    cv::cornerHarris(roic,roic2,11,3,0.04);
    for (uint k=0; k<blobs.size(); k++){
        cv::circle(roi, blobs[k].pt, 5, cv::Scalar(255,0,0), -2);
    }
//    cv::imshow("ROI",roi);
////    cv::imshow("ROIC",roic2);
//    cv::waitKey(0);

    // get all possible combinations of 3 points
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
                id = decode(roi, points);
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
    std::vector<cv::Point3f> points(3);
    points[0] = cv::Point3f(0,_dimension.y*0.5,0);
    points[1] = cv::Point3f(0,-_dimension.y*0.5,0);
    points[2] = cv::Point3f(_dimension.x,0,0);

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
