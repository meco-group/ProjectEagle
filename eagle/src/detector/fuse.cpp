#include "fuse.h"

using namespace eagle;

void eagle::remapinf(std::string config, const cv::Mat& img, cv::Mat& warped, const float k, const cv::Size& siz, double z0) {
    Projection projection(config);
    remapinf(projection, img, warped, k, siz, z0);
}

void eagle::remapinf(const Projection& projection, const cv::Mat& img, cv::Mat& warped, const float k, const cv::Size& siz, double z0) {
    // initialize parameters
    cv::Size ksiz = cv::Size(siz.width * k, siz.height * k);
    cv::Point2f offset(ksiz.height*0.5,ksiz.width*0.5);

    // Do remapping
    remapinf_canvas(projection, img, warped, k, offset, ksiz, z0);
}

void eagle::remapinf_cropped(std::string config, const cv::Mat& img, cv::Mat& warped, const float k, cv::Point2f& offset, double z0) {
    Projection projection(config);
    remapinf_cropped(projection, img, warped, k, offset, z0);
}

void eagle::remapinf_cropped(const Projection& projection, const cv::Mat& img, cv::Mat& warped, const float k, cv::Point2f& offset, double z0) {
    // Init projection object
    cv::Mat S = (cv::Mat_<float>(3, 3) << k, 0, 0, 0, k, 0, 0, 0, 1);
    cv::Mat H = S * projection.get_homography(z0);

    // Define corner points
    std::vector<cv::Point2f> corners(4);
    corners[0] = cv::Point2f(0,0);
    corners[1] = cv::Point2f(img.cols,0);
    corners[2] = cv::Point2f(0,img.rows);
    corners[3] = cv::Point2f(img.cols,img.rows);
    
    std::vector<cv::Point2f> transformed;
    cv::perspectiveTransform(corners,transformed,H);

    cv::Rect r = cv::boundingRect(transformed);
    cv::Size siz(r.width, r.height);
    offset = -cv::Point2f(r.x, r.y);

    // Do remapping
    remapinf_canvas(projection, img, warped, k, offset, siz, z0);
}

void eagle::remapinf_canvas(const Projection& projection, const cv::Mat& img, cv::Mat& warped, const float k, cv::Point2f& offset, cv::Size& siz, double z0) {
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

void eagle::overlay(const cv::Mat& in1, const cv::Point2f& offset1, const cv::Mat& in2, const cv::Point2f& offset2, cv::Mat& out) {
    // compute canvases
    cv::Rect rect1(out.cols*0.5-offset1.x, out.rows*0.5-offset1.y, in1.cols, in1.rows);
    cv::Rect rect2(out.cols*0.5-offset2.x, out.rows*0.5-offset2.y, in2.cols, in2.rows);

    // check size of output image
    cv::Rect c(0,0,out.cols,out.rows);
    if (!(c.contains(rect1.tl()) && c.contains(rect1.br()) && c.contains(rect2.tl()) && c.contains(rect2.br()))) {
        if ((c.width != 0) && (c.height != 0)) {
            std::cout << "provided size insufficient: changing output size" << std::endl;
        }
        
        // compute theoretical positions
        float xmin = std::min(-offset1.x, -offset2.x);
        float ymin = std::min(-offset1.y, -offset2.y);
        float xmax = std::max(-offset1.x + in1.cols, -offset2.x + in2.cols);
        float ymax = std::max(-offset1.y + in1.rows, -offset2.y + in2.rows);
        
        // change rect1 and 2 to available area
        rect1 = cv::Rect(-xmin-offset1.x, -ymin-offset1.y, in1.cols, in1.rows);
        rect2 = cv::Rect(-xmin-offset2.x, -ymin-offset2.y, in2.cols, in2.rows);

        // change output image size
        out = cv::Mat(ymax-ymin, xmax-xmin, CV_8UC3);
    }

    // copy the images to bigger versions
    cv::Mat in1_ = cv::Mat::zeros(out.rows, out.cols, CV_8UC3);
    cv::Mat roi(in1_,rect1);
    in1.copyTo(roi);
    cv::Mat in2_ = cv::Mat::zeros(out.rows, out.cols, CV_8UC3);
    roi = cv::Mat(in2_,rect2);
    in2.copyTo(roi);

    // do the overlay of two images
    overlay(in1_, in2_, out);
}

void eagle::replace(const cv::Mat& in, const cv::Point2f& offset, cv::Mat& out, double weight) {
    cv::Rect rect(out.cols*0.5-offset.x, out.rows*0.5-offset.y, in.cols, in.rows);
    cv::Mat big = cv::Mat::zeros(out.rows, out.cols, in.type());
    cv::Mat roi(big,rect);
    in.copyTo(roi);

    replace(big,out,weight);
}

void eagle::replace(const cv::Mat& in, cv::Mat& out, double weight) {
    cv::Mat c;
    out.copyTo(c);
    in.copyTo(c, in);
    cv::addWeighted(c, weight, out, 1.0 - weight, 0.0, out);
}

