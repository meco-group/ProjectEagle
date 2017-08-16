#ifndef V4L2_CAMERA_H
#define V4L2_CAMERA_H

#include "camera_interface.h"
#include <linux/videodev2.h>
#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

namespace eagle {

    typedef struct buffer_t {
        void *data;
        size_t length;
    } buffer_t;

    class V4L2Camera : public Camera {

        private:
            int _fd;
            int _width;
            int _height;
            int _pixelformat;
            struct timeval _capture_time;
            int _buffercount;

            int xioctl(int fd, int request, void *arg);
            int v4l2_set_input();
            int v4l2_set_pixfmt();
            int v4l2_set_buffer();
            int v4l2_start_capture();
            int v4l2_dequeue_buffer();
            int v4l2_queue_buffer();
            int v4l2_stop_capture();
            virtual int process_buffer(cv::Mat &img) = 0;

        protected:
            v4l2_buffer _bufferinfo;
            buffer_t *_buffers;

            int v4l2_set_brightness(unsigned int brightness);
            int v4l2_set_exposure(unsigned int exposure);
            int v4l2_set_iso(unsigned int iso);

        public:
            V4L2Camera(int device = 0);
            bool start();
            bool stop();
            bool isOpened();
            virtual bool read(cv::Mat &img);
            void buffers(int buffercount);
            void format(int width, int height, int pixelformat);
            void setResolution(int width, int height);
            int getWidth();
            int getHeight();
            void *getBuffer();
            int getBufferLength();
            double getV4L2CaptureTime();

    };

};

#endif //V4L2_CAMERA_H
