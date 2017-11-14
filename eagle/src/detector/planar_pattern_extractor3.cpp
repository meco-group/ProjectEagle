#include "planar_pattern_extractor3.h"

using namespace eagle;

PlanarPatternExtractor3::PlanarPatternExtractor3(const cv::String& config) :
    PatternExtractor3(config), _projection(config) {
    cv::FileStorage fs(config, cv::FileStorage::READ);
    fs["camera"]["ground_plane"] >> _plane;
    fs.release();
}

PlanarPatternExtractor3::PlanarPatternExtractor3(const Pattern& pattern, const Projection& projection, const cv::Mat& plane) :
    PatternExtractor3(pattern), _projection(projection), _plane(plane) {

}

cloud3_t PlanarPatternExtractor3::extract(cv::Mat& img, const cv::Point2f& offset, int& id, bool display) {
    // undistort image via projection
    cv::Mat undist;
    _projection.remap(img, undist);
    cloud2_t image_points = _pattern_extractor.extract(undist, offset, id, display);

    // map image coordinates to the world
    cloud3_t world_points;
    if (!image_points.empty()) {
        world_points = _projection.project_to_plane(image_points, _plane);
    }

    return world_points;
}

void PlanarPatternExtractor3::set_transform(const cv::Mat& T) {
    _projection.set_transform(T);
}
