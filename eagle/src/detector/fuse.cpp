#include "fuse.h"

void eagle::remapinf(std::string config, const cv::Mat& img, cv::Mat& warped, uint r, const cv::Size& siz, double z0) {
    // Init projection object
    std::cout << config << std::endl;
    Projection projection(config);
    cv::Size rsiz = cv::Size(siz.width * r, siz.height * r);

    // Rectify image -> already done by eagle

    // Construct homography matrix
    cv::Mat S = (cv::Mat_<float>(3, 3) << r, 0, rsiz.width * 0.5, 0, r, rsiz.height * 0.5, 0, 0, 1);
    cv::Mat H = S * projection.get_homography(z0);

    // Warp image
    cv::warpPerspective(img, warped, H, rsiz);
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

