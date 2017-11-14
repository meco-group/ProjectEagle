#include "transform.h"

cv::Mat eagle::compute_transform(std::vector<std::vector<cv::Point3f>>& points1, std::vector<std::vector<cv::Point3f>>& points2) {
    cv::Mat P1(0, 3, CV_32FC1);
    cv::Mat P2(0, 3, CV_32FC1);
    for (uint k = 0; k < points1.size(); k++) {
        if ((!points1[k].empty()) && (!points2[k].empty())) {
            cv::vconcat(P1, cv::Mat(points1[k].size(), 3, CV_32FC1, points1[k].data()), P1);
            cv::vconcat(P2, cv::Mat(points2[k].size(), 3, CV_32FC1, points2[k].data()), P2);
        } else {
            std::cout << "point cloud " << k << " invalid" << std::endl;
        }
    }
    P1 = P1.t();
    P2 = P2.t();

    return compute_transform(P1, P2);
}

cv::Mat eagle::compute_transform(std::vector<cv::Point3f>& points1, std::vector<cv::Point3f>& points2) {
    cv::Mat P1(points1.size(), 3, CV_32FC1, points1.data());
    cv::Mat P2(points2.size(), 3, CV_32FC1, points2.data());

    P1 = P1.t();
    P2 = P2.t();

    return compute_transform(P1, P2);
}

/*
 * The transform is so that camera1 is the base and camera2 is expressed in the frame of camera1
 * p1 = R*p2+t
 * taken from: http://nghiaho.com/?page_id=671
 */

cv::Mat eagle::compute_transform(const cv::Mat& points1, const cv::Mat& points2) {
    // copy input matrices.
    cv::Mat P1 = points1.clone();
    cv::Mat P2 = points2.clone();

    // compute centroids
    cv::Mat centroid1, centroid2;
    reduce(P1, centroid1, 1, CV_REDUCE_AVG);
    reduce(P2, centroid2, 1, CV_REDUCE_AVG);

    // shift point clouds w.r.t. centroids
    P1 = P1 - repeat(centroid1, 1, P1.cols);
    P2 = P2 - repeat(centroid2, 1, P2.cols);

    // compute svd of P1*P2'
    cv::Mat H = P2 * (P1.t());
    cv::Mat U, S, Vt;
    cv::SVD::compute(H, S, U, Vt);

    // compute translation and rotation
    cv::Mat R, t;
    R = (U * Vt).t();
    if (cv::determinant(R) < 0) {
        cv::Mat c = R.col(2) * (-1.0f);
        c.copyTo(R.col(2));
    }
    t = -R * centroid2 + centroid1;

    // Construct transformation matrix
    cv::Mat h1, h2, T21;
    cv::hconcat(R, t, h1);
    cv::hconcat(cv::Mat::zeros(1, 3, CV_32FC1), cv::Mat::ones(1, 1, CV_32FC1), h2);
    cv::vconcat(h1, h2, T21);

    return T21;
}
cv::Mat eagle::compute_inplane_transform(const cv::Point3f& n1) {
    cv::Point3f ez = n1;
    ez /= cv::norm(ez);
    cv::Point3f ex(ez.z, 0, -ez.x);
    ex /= cv::norm(ex);
    cv::Point3f ey = ez.cross(ex);
    ey /= cv::norm(ey);

    std::vector<cv::Point3f> unitvectors {ex, ey, ez};
    cv::Mat R(unitvectors.size(), 3, CV_32FC1, unitvectors.data());
    //R = R.t();

    cv::Mat T = cv::Mat::eye(4, 4, CV_32F);
    R.copyTo(T(cv::Rect(0, 0, 3, 3)));

    return T;
}

cv::Mat eagle::compute_inplane_transform(const cv::Mat& plane) {
    cv::Mat t;
    plane.convertTo(t, CV_32F);
    cv::Point3f n(t(cv::Rect(0, 0, 3, 1)));
    return compute_inplane_transform(n);
}

cv::Mat eagle::get_rotation(const cv::Mat& T) {
    return T(cv::Rect(0, 0, 3, 3)).clone();
}

/*
 * taken from: https://www.learnopencv.com/rotation-matrix-to-euler-angles/
 */
cv::Point3f eagle::get_euler(const cv::Mat& T) {
    cv::Mat R = get_rotation(T); //get part of T or R immediately
    R.convertTo(R, CV_64F);

    float sy = sqrt(R.at<double>(0, 0) * R.at<double>(0, 0) +  R.at<double>(1, 0) * R.at<double>(1, 0) );
    bool singular = sy < 1e-6; // If

    double x, y, z;
    if (!singular) {
        x = std::atan2(R.at<double>(2, 1) , R.at<double>(2, 2));
        y = std::atan2(-R.at<double>(2, 0), sy);
        z = std::atan2(R.at<double>(1, 0), R.at<double>(0, 0));
    } else {
        x = std::atan2(-R.at<double>(1, 2), R.at<double>(1, 1));
        y = std::atan2(-R.at<double>(2, 0), sy);
        z = 0;
    }

    return cv::Point3f(x, y, z);
}

cv::Mat eagle::get_translation(const cv::Mat& T) {
    return T(cv::Rect(3, 0, 1, 3)).clone();
}

cv::Point3f eagle::transform(const cv::Mat& T, const cv::Point3f& p) {
    return cv::Point3f(cv::Mat(get_rotation(T) * cv::Mat(p) + get_translation(T)));
}
std::vector<cv::Point3f> eagle::transform(const cv::Mat& T, const std::vector<cv::Point3f>& points) {
    std::vector<cv::Point3f> pt; pt.reserve(points.size());
    for (uint k = 0; k < points.size(); k++) {
        pt.push_back(transform(T, points[k]));
    }

    return pt;
}
std::vector<std::vector<cv::Point3f>> eagle::transform(const cv::Mat& T, const std::vector<std::vector<cv::Point3f>>& points) {
    std::vector<std::vector<cv::Point3f>> pt; pt.reserve(points.size());
    for (uint k = 0; k < points.size(); k++) {
        pt.push_back(transform(T, points[k]));
    }

    return pt;
}

cv::Point2f eagle::transform2(const cv::Mat& T, const cv::Point3f& p) {
    return dropz(transform(T, p));
}

std::vector<cv::Point2f> eagle::transform2(const cv::Mat& T, const std::vector<cv::Point3f>& points) {
    std::vector<cv::Point2f> pt; pt.reserve(points.size());
    for (uint k = 0; k < points.size(); k++) {
        pt.push_back(transform2(T, points[k]));
    }

    return pt;
}

std::vector<std::vector<cv::Point2f>> eagle::transform2(const cv::Mat& T, const std::vector<std::vector<cv::Point3f>>& points) {
    std::vector<std::vector<cv::Point2f>> pt; pt.reserve(points.size());
    for (uint k = 0; k < points.size(); k++) {
        pt.push_back(transform2(T, points[k]));
    }

    return pt;
}

cv::Point2f eagle::dropz(const cv::Point3f& p) {
    return cv::Point2f(p.x, p.y);
}

std::vector<cv::Point2f> eagle::dropz(const std::vector<cv::Point3f>& points) {
    std::vector<cv::Point2f> pt; pt.reserve(points.size());
    for (uint k = 0; k < points.size(); k++) {
        pt.push_back(dropz(points[k]));
    }

    return pt;
}

std::vector<std::vector<cv::Point2f>> eagle::dropz(const std::vector<std::vector<cv::Point3f>>& points) {
    std::vector<std::vector<cv::Point2f>> pt; pt.reserve(points.size());
    for (uint k = 0; k < points.size(); k++) {
        pt.push_back(dropz(points[k]));
    }

    return pt;

}

cv::Point3f eagle::addz(const cv::Point2f& p, float z) {
    return cv::Point3f(p.x, p.y, z);
}

std::vector<cv::Point3f> eagle::addz(const std::vector<cv::Point2f>& points, float z) {
    std::vector<cv::Point3f> p(points.size());
    for (uint k = 0; k < points.size(); k++) {
        p[k] = addz(points[k], z);
    }

    return p;
}

std::vector<std::vector<cv::Point3f>> eagle::addz(const std::vector<std::vector<cv::Point2f>>& points, float z) {
    std::vector<std::vector<cv::Point3f>> p(points.size());
    for (uint k = 0; k < points.size(); k++) {
        p[k] = addz(points[k], z);
    }

    return p;

}

cv::Mat eagle::transform_plane(const cv::Mat& T, const cv::Mat& plane) {
    cv::Mat p, t;
    plane.convertTo(p, CV_32F);
    T.convertTo(t, CV_32F);
    return p * t;
}
