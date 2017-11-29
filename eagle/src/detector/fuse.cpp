#include "fuse.h"

using namespace eagle;

void eagle::remapinf(std::string config, const cv::Mat& img, cv::Mat& warped, const float k, const cv::Size& siz, double z0) {
    // Init projection object
    Projection projection(config);

    cv::Size ksiz = cv::Size(siz.width * k, siz.height * k);
    cv::Point2f offset(ksiz.height*0.5,ksiz.width*0.5);

    // Do remapping
    remapinf_canvas(projection, img, warped, k, offset, ksiz, z0);
}

void eagle::remapinf_cropped(std::string config, const cv::Mat& img, cv::Mat& warped, const float k, cv::Point2f& offset, double z0) {
    // Init projection object
    Projection projection(config);

    std::vector<cv::Point2f> corners(4);
    corners[0] = cv::Point2f(0,0);
    corners[1] = cv::Point2f(img.cols,0);
    corners[2] = cv::Point2f(0,img.rows);
    corners[3] = cv::Point2f(img.cols,img.rows);

    cv::Mat plane = (cv::Mat_<float>(1,4) << 0,0,1,-z0);
    cv::Point3f t;
    for (uint k=0; k<corners.size(); k++) {
        t = projection.project_to_plane(corners[k], plane);
        corners[k] = cv::Point2f(t.x,t.y);
    }
    cv::Rect rect = cv::boundingRect(corners);

    cv::Size siz(rect.width, rect.height);
    offset = cv::Point2f(rect.x*k,rect.y*k);

    // Do remapping
    remapinf_canvas(projection, img, warped, k, offset, siz, z0);
}

void eagle::remapinf_canvas(Projection& projection, const cv::Mat& img, cv::Mat& warped, const float k, cv::Point2f& offset, cv::Size& siz, double z0) {
    // Construct homography matrix
    // offset and siz in px
    cv::Mat S = (cv::Mat_<float>(3, 3) << k, 0, offset.x, 0, k, offset.y, 0, 0, 1);
    cv::Mat H = S * projection.get_homography(z0);

    // Warp image
    cv::warpPerspective(img, warped, H, siz);
}

void eagle::overlay(const cv::Mat& in1, const cv::Mat& in2, cv::Mat& out) {
    cv::Mat in1_grey, in2_grey, union_mask, union_im;
    std::vector<std::vector<cv::Point2i>> contours;
    cv::cvtColor(in1, in1_grey, CV_BGR2GRAY);
    cv::findContours(in1_grey, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    cv::fillPoly(in1_grey, contours, cv::Scalar(127,127,127));
    cv::cvtColor(in2, in2_grey, CV_BGR2GRAY);
    cv::findContours(in2_grey, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    cv::fillPoly(in2_grey, contours, cv::Scalar(127,127,127));
    cv::bitwise_and(in1_grey, in2_grey, union_mask);
    cv::addWeighted(in1, 0.5, in2, 0.5, 0, union_im);

    cv::bitwise_or(in1, in2, out); //compose by overlaying
    union_im.copyTo(out, union_mask);
}

void eagle::replace(const cv::Mat& in, cv::Mat& out, double weight) {
    cv::Mat c;
    out.copyTo(c);
    in.copyTo(c, in);
    cv::addWeighted(c, weight, out, 1.0 - weight, 0.0, out);
}

