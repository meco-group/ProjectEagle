#include <pnp_pattern_extractor3.h>

using namespace eagle;

PnpPatternExtractor3::PnpPatternExtractor3(const cv::String& config) :
    PatternExtractor3(config)
{
    cv::FileStorage fs(config, cv::FileStorage::READ);
    fs["camera"]["camera_matrix"] >> _camera_matrix;
    fs["camera"]["distortion_vector"] >> _distortion_vector;
    fs["camera"]["external_transformation"] >> _T;
    _T.convertTo(_T,CV_32F);
    fs.release();
}

PnpPatternExtractor3::PnpPatternExtractor3(const Pattern& pattern, const cv::Mat& camera_matrix, const cv::Mat& distortion_vector, const cv::Mat& T) :
    PatternExtractor3(pattern), _camera_matrix(camera_matrix), _distortion_vector(distortion_vector), _T(T)
{

}

cloud3_t PnpPatternExtractor3::extract(cv::Mat& img, const cv::Point2f& offset, int& id, bool display) {
    //extract pattern image points
    cloud2_t image_points = _pattern_extractor.extract(img, offset, id, display);

    // map image coordinates to the world
    cloud3_t world_points;
    if (!image_points.empty()) {
        cloud3_t object_points = _pattern_extractor.pattern().reference();
        cv::Mat rvec, tvec;
        cv::solvePnP(object_points, image_points, _camera_matrix, _distortion_vector, rvec, tvec);

        // ugly mathematics to get the types right
        cv::Mat R;
        cv::Rodrigues(rvec, R);
        cv::Mat temp;
        R.convertTo(R, CV_32F);
        tvec.convertTo(tvec, CV_32F);
        
        cloud3_t camera_points;
        camera_points.reserve(image_points.size());
        for (uint k=0; k<object_points.size(); k++) {
            //do the transformation
            temp = R*cv::Mat(object_points[k]) + tvec;
            camera_points.push_back(cv::Point3f(temp));
        }
        world_points = transform(_T, camera_points);
    }

    return world_points;
}

void PnpPatternExtractor3::set_transform(const cv::Mat& T) {
    T.convertTo(_T,CV_32F);
}
