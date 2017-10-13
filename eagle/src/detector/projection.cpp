#include <projection.h>
#include <transform.h>

using namespace eagle;

void Projection::init(const cv::Mat& camera_matrix, const cv::Mat& distortion_vector, const cv::Size& size, const cv::Mat& T) {
    _camera_matrix = cv::getOptimalNewCameraMatrix(camera_matrix, distortion_vector, size, 1);
    cv::initUndistortRectifyMap(camera_matrix, distortion_vector, cv::Mat::eye(3,3,CV_32F), 
        _camera_matrix, size, CV_16SC2, _intrinsic_map1, _intrinsic_map2);
    _camera_matrix.convertTo(_camera_matrix,CV_32F);
    set_transform(T);
}

Projection::Projection(const cv::Mat& camera_matrix, const cv::Mat& distortion_vector, const cv::Size& size, const cv::Mat& T)
{
    init(camera_matrix, distortion_vector, size, T);
}

Projection::Projection(const cv::Mat& camera_matrix, const cv::Mat& distortion_vector, const cv::Mat& sample, const cv::Mat& T) {
    init(camera_matrix, distortion_vector, sample.size(), T);
}

Projection::Projection(const cv::String& config) {
    cv::FileStorage fs(config, cv::FileStorage::READ);
    cv::Mat camera_matrix, distortion_vector, T;
    fs["camera"]["camera_matrix"] >> camera_matrix;
    fs["camera"]["distortion_vector"] >> distortion_vector;
    fs["camera"]["external_transformation"] >> T;
    int width = fs["camera"]["resolution"]["width"];
    int height = fs["camera"]["resolution"]["height"];
    fs.release();
    init(camera_matrix, distortion_vector, cv::Size(width,height), T); // opencv size returns cols,rows!
}
void Projection::remap(const cv::Mat& origin, cv::Mat& destination) {
    cv::remap(origin, destination, _intrinsic_map1, _intrinsic_map2, cv::INTER_LINEAR);
}

cv::Point3f Projection::project_to_plane(const cv::Point3f& i, const cv::Mat& plane) {
    // Project image coordinates to a plane
    // i is of the form [x;y;1], w is of the form [X,Y,Z]
    // plane holds the coefficients of the plane [a,b,c,d]
    // which is represented by ax+by+cz+d=0

    cv::Mat M1, M2, A, i_m, pl, K;
    _camera_matrix.convertTo(K,CV_64F);
    plane.convertTo(pl,CV_64F);
    cv::Mat(i).convertTo(i_m, CV_64F);
    cv::hconcat(-cv::Mat::eye(3,3,CV_64F), K.inv()*i_m, M1);
    cv::hconcat(pl(cv::Rect(0,0,3,1)), cv::Mat::zeros(1, 1, CV_64F), M2);
    cv::vconcat(M1, M2, A);
    cv::Mat b = cv::Mat::zeros(4,1,CV_64F);
    b.at<double>(3, 0) = -pl.at<double>(0, 3);
    cv::Mat W = A.inv()*b;
    cv::Point3f w(W(cv::Rect(0, 0, 1, 3)));
    return transform(_T, w);
}

cv::Point3f Projection::project_to_plane(const cv::Point2f& i, const cv::Mat& plane) {
    cv::Point3f i3 = cv::Point3f(i.x, i.y, 1.0f);
    return project_to_plane(i3, plane);
}

std::vector<cv::Point3f> Projection::project_to_plane(const std::vector<cv::Point3f> i, const cv::Mat& plane) {
    std::vector<cv::Point3f> w; w.reserve(i.size());
    for (uint k=0; k<i.size(); k++) {
        w.push_back(project_to_plane(i[k], plane));
    }

    return w;
}

std::vector<cv::Point3f> Projection::project_to_plane(const std::vector<cv::Point2f> i, const cv::Mat& plane) {
    std::vector<cv::Point3f> w; w.reserve(i.size());
    for (uint k=0; k<i.size(); k++) {
        w.push_back(project_to_plane(i[k], plane));
    }

    return w;
}

std::vector<std::vector<cv::Point3f>> Projection::project_to_plane(const std::vector<std::vector<cv::Point3f>> i, const cv::Mat& plane) {
    std::vector<std::vector<cv::Point3f>> w; w.reserve(i.size());
    for (uint k=0; k<i.size(); k++) {
        w.push_back(project_to_plane(i[k], plane));
    }

    return w;
}

std::vector<std::vector<cv::Point3f>> Projection::project_to_plane(const std::vector<std::vector<cv::Point2f>> i, const cv::Mat& plane) {
    std::vector<std::vector<cv::Point3f>> w; w.reserve(i.size());
    for (uint k=0; k<i.size(); k++) {
        w.push_back(project_to_plane(i[k], plane));
    }

    return w;
}

cv::Point3f Projection::project_to_image3(const cv::Point3f& w) {
    cv::Point3f c = transform(_T.inv(), w);
    cv::Mat p = _camera_matrix*cv::Mat(c);
    p = p*(1.0/p.at<float>(2, 0));
    return cv::Point3f(p.at<float>(0, 0), p.at<float>(1, 0), p.at<float>(2, 0));
}

cv::Point2f Projection::project_to_image(const cv::Point3f& w) {
    cv::Point3f i3 = project_to_image3(w);
    return cv::Point2f(i3.x, i3.y);
}
std::vector<cv::Point3f> Projection::project_to_image3(const std::vector<cv::Point3f>& w) {
    std::vector<cv::Point3f> p; p.reserve(w.size());
    for (uint k=0; k<w.size(); k++) {
        p.push_back(project_to_image3(w[k]));
    }

    return p;
}
std::vector<cv::Point2f> Projection::project_to_image(const std::vector<cv::Point3f>& w) {
    std::vector<cv::Point2f> p; p.reserve(w.size());
    for (uint k=0; k<w.size(); k++) {
        p.push_back(project_to_image(w[k]));
    }

    return p;
}

void Projection::set_transform(const cv::Mat& T) {
    T.convertTo(_T,CV_32F);
}
