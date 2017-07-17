#ifndef OPI_CAMERA_H
#define OPI_CAMERA_H

#include "v4l2_camera.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace cam {
    class OPICamera : public V4L2Camera {
    private:
        int process_buffer(cv::Mat &img);

        bool readUYVY(cv::Mat &img);

    public:
        OPICamera(int device = 0);

    };
}
#endif //OPI_CAMERA_H
