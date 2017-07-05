
#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <signal.h>
#include <assert.h>
#include <getopt.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include "cam/v4l2_camera.h"

V4L2Camera::V4L2Camera(int device):
    CameraInterface(0),
    _width(640),
    _height(480),
    _pixelformat(V4L2_PIX_FMT_YUYV),
	_buffercount(4)
{
    // device should be converted with to_string, but this is not compiling
    char d[3];
    sprintf(d, "%d", device);
    std::string device_name = "/dev/video" + std::string(d);
    if((_fd = open(device_name.c_str(), O_RDWR)) < 0){
        perror("open");
    }
}

int V4L2Camera::xioctl(int fd, int request, void *arg)
{
	int r;
	do r = ioctl (fd, request, arg);
	while (-1 == r && EINTR == errno);
	return r;
}

int V4L2Camera::v4l2_set_input()
{
	/*
	 * define video input
	 * vfe_v4l2 the driver is forced to input = -1
	 * set as the input = 0, works fine.
	 */
	struct v4l2_input input;
	int count = 0;
	CLEAR(input);
	input.index = count;
	while(!ioctl(_fd, VIDIOC_ENUMINPUT, &input)) {
		input.index = ++count;
	}
	count -= 1;

	assert(count > -1);

	if(xioctl(_fd, VIDIOC_S_INPUT, &count) == -1) {
		printf("Error selecting input %d\n", count);
		return 1;
	}
	return 0;
}

int V4L2Camera::v4l2_set_pixfmt()
{
	/*
	 * define pixel format
	 * in gc2035 driver, tested with:
	 * -- 422P/YU12/YV12/NV16/NV12/NV61/NV21/UYVY
	 * others will be ignored
	 *
	 */
	struct v4l2_format fmt;
	CLEAR(fmt);

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = _width;        // ignored by gc2035 driver
	fmt.fmt.pix.height = _height;      // ignored by gc2035 driver
	fmt.fmt.pix.pixelformat = _pixelformat;
	fmt.fmt.pix.field = V4L2_FIELD_ANY;
	if (-1 == xioctl(_fd, VIDIOC_S_FMT, &fmt)) {
		perror("Setting Pixel Format");
		return 1;
	}
	printf("Selected Camera Mode:\n"
			"  Width: %d\n"
			"  Height: %d\n"
			"  PixFmt: %s\n",
			fmt.fmt.pix.width,
			fmt.fmt.pix.height,
			(char *)&fmt.fmt.pix.pixelformat);

	if (_pixelformat != fmt.fmt.pix.pixelformat) {
		_pixelformat = fmt.fmt.pix.pixelformat;
        perror("Pixel format not accepted");
		return 1;
	}
	if ((_height != fmt.fmt.pix.height) || (_width != fmt.fmt.pix.width)) {
	    _width = fmt.fmt.pix.width;
	    _height = fmt.fmt.pix.height;
        perror("Image size not accepted");
		return 1;
	}
	return 0;
}

int V4L2Camera::v4l2_set_brightness(unsigned int brightness)
{
    struct v4l2_control control;
    control.id = V4L2_CID_BRIGHTNESS;
    control.value = brightness;
    if (xioctl(_fd, VIDIOC_S_CTRL, &control) == -1){
        std::cout << "Error while setting the brighness!" << std::endl;
        return 1;
    }
    return 0;
}

int V4L2Camera::v4l2_set_exposure(unsigned int exposure)
{
    struct v4l2_control control;
    control.id = V4L2_CID_EXPOSURE_AUTO;
    control.value = 0; // manual
    if (xioctl(_fd, VIDIOC_S_CTRL, &control) == -1){
        std::cout << "Error while setting the exposure to manual!" << std::endl;
        return 1;
    }
    control.id = V4L2_CID_EXPOSURE;
    control.value = exposure;
    if (xioctl(_fd, VIDIOC_S_CTRL, &control) == -1){
        std::cout << "Error while setting the exposure!" << std::endl;
    }
    return 0;
}

int V4L2Camera::v4l2_set_iso(unsigned int iso)
{
    struct v4l2_control control;
    control.id = V4L2_CID_ISO_SENSITIVITY_AUTO;
    control.value = 0; // manual
    if (xioctl(_fd, VIDIOC_S_CTRL, &control) == -1){
        std::cout << "Error while setting the ISO to manual!" << std::endl;
        return 1;
    }
    control.id = V4L2_CID_ISO_SENSITIVITY;
    control.value = iso;
    if (xioctl(_fd, VIDIOC_S_CTRL, &control) == -1){
        std::cout << "Error while setting the iso!" << std::endl;
    }
    return 0;
}

int V4L2Camera::v4l2_set_buffer()
{
	// Inform about future buffers
	struct v4l2_requestbuffers bufrequest;
	bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	bufrequest.memory = V4L2_MEMORY_MMAP;
	bufrequest.count = _buffercount; //1;

	if(xioctl(_fd, VIDIOC_REQBUFS, &bufrequest) < 0){
	    perror("VIDIOC_REQBUFS");
		return 1;
	}

	// Allocate the vector with bufferinfo
	_buffers = new buffer_t[_buffercount];

	// Allocate the buffers
	for (int i = 0; i < _buffercount; i++) {
		struct v4l2_buffer _bufferinfo;
		CLEAR(_bufferinfo);

		_bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		_bufferinfo.memory = V4L2_MEMORY_MMAP;
		_bufferinfo.index = i;

		if(xioctl(_fd, VIDIOC_QUERYBUF, &_bufferinfo) < 0){
			perror("VIDIOC_QUERYBUF");
			return 1;
		}

		// Make the buffer
		_buffers[i].length = _bufferinfo.length;
		_buffers[i].data = mmap(
			NULL,
		    _bufferinfo.length,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			_fd,
			_bufferinfo.m.offset
		);

		if(_buffers[i].data == MAP_FAILED){
			perror("mmap");
			return 1;
		}
		memset(_buffers[i].data, 0, _buffers[i].length);
	}

	return 0;
}

int V4L2Camera::v4l2_start_capture()
{
	//queue the available buffers
	for (int i = 0; i < _buffercount; i++) {
		CLEAR(_bufferinfo);
		_bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		_bufferinfo.memory = V4L2_MEMORY_MMAP;
		_bufferinfo.index = i; /* Queueing buffer i. */

		if (xioctl(_fd, VIDIOC_QBUF, &_bufferinfo) < 0) {
			perror("VIDIOC_QBUF");
			return 1;
		}
	}

	// Activate streaming
    if(xioctl(_fd, VIDIOC_STREAMON, &_bufferinfo.type) < 0){
	    perror("VIDIOC_STREAMON");
		return 1;
	}

    return 0;
}

int V4L2Camera::v4l2_stop_capture()
{
	if(xioctl(_fd, VIDIOC_STREAMOFF, &_bufferinfo.type) < 0){
	    perror("VIDIOC_STREAMOFF");
		return 1;
	}

	close(_fd);
	_fd = -1;

    return 0;
}

int V4L2Camera::v4l2_dequeue_buffer()
{
	CLEAR(_bufferinfo);
	_bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	_bufferinfo.memory = V4L2_MEMORY_MMAP;

	// The buffer's waiting in the outgoing queue.
	if(xioctl(_fd, VIDIOC_DQBUF, &_bufferinfo) < 0){
		perror("VIDIOC_DQBUF");
		return 1;
	}

    _capture_time = _bufferinfo.timestamp;

	return 0;
}

int V4L2Camera::v4l2_queue_buffer()
{
	// Put the buffer in the incoming queue.
	if(xioctl(_fd, VIDIOC_QBUF, &_bufferinfo) < 0){
		perror("VIDIOC_QBUF");
		return 1;
	}

	return 0;
}

bool V4L2Camera::start()
{
    v4l2_set_input();
    v4l2_set_pixfmt();
    v4l2_set_buffer();
    v4l2_start_capture();
}

bool V4L2Camera::read(cv::Mat &img)
{
    cv::Mat img_;
	v4l2_dequeue_buffer();
	process_buffer(img_);
	v4l2_queue_buffer();
    cv::undistort(img_, img, _camera_matrix, _distortion_vector);
	return true;
}

bool V4L2Camera::stop()
{
    return v4l2_stop_capture() == 0;
}

void V4L2Camera::format(int width, int height, int pixelformat)
{
    _width = width;
    _height = height;
    _pixelformat = pixelformat;
}

void V4L2Camera::buffers(int buffercount)
{
	assert(buffercount > 0);
	_buffercount = buffercount;
}

bool V4L2Camera::isOpened()
{
    return _fd>0;
}

int V4L2Camera::getWidth()
{
    return _width;
}

int V4L2Camera::getHeight()
{
    return _height;
}

void* V4L2Camera::getBuffer()
{
	return _buffers[_bufferinfo.index].data;
}

int V4L2Camera::getBufferLength()
{
    return _buffers[_bufferinfo.index].length;
}

double V4L2Camera::getV4L2CaptureTime()
{
    return (double)_capture_time.tv_sec + _capture_time.tv_usec*1e-6;
}
