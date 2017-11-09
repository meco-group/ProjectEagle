# Project Eagle

Project Eagle collects modular building blocks for obstacle and robot pose detection using a camera.

## Building the project

### Dependencies
Install the following packages:
* cmake version 3.1 or higher
    - check this in the terminal with the command `cmake --version`
    - if not installed, get cmake from the website
* OpenCV 3.2
    - Download source from [here](https://github.com/opencv/opencv/releases).
    - `mkdir build` and `cd build`
    - `cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local ..`
    - `make` and `sudo make install`
* `zyre`, `zmq`, `czmq` and `libsodium`, see [this](https://github.com/zeromq/zyre) for installation instructions.

### Building and installing the project
Starting from the root of the repository:
```
mkdir build
cd build
cmake ..
make
sudo make install
```
Built examples are in `build/bin/`

### Linking Project Eagle with your application
When using `cmake`, add the following to your `CMakeLists.txt`:
```
find_package(eagle REQUIRED)
include_directories(${eagle_INCLUDE_DIRS})
target_link_libraries(MyTarget ${eagle_LIBRARIES})
```

## Detector

The detector detects obstacles and robots in the environment using a captured camera frame. Obstacles and robots are distinguished from the environment by comparing the captured frame by a capture of the background. Robots are identified by using a proper marker. The figure underneath gives an overview of a marker with its major dimensions.

<div align="center">
<img src="doc/drawing.png" align="middle" width="40%" alt="fancy pancy marker"/>
</div>

A marker is defined by the following parameters (see `detector.yml`):
* `triangle_ratio`: w/h
* `qr_posx`: qr_x/w
* `qr_posy`: qr_y/h
* `qr_sizex`: qr_w/w
* `qr_sizey`: qr_h/h
* `qr_nbitx`: number of bits in x direction
* `qr_nbity`: number of bits in y direction

A robot's pose is determined using the 3 black disks. In the source code, these markers are always ordered as a, b and c.
The robot's id is determined by decoding the `qr_nbitx`*`qr_nbity` bit QR code in the center of the marker. The value of each bit is illustrated in the figure for a 4 bit QR. A black square means 1, a white square equals 0.

## Generic V4L2 Camera Drivers

### What is it?

OpenCV provides a marvelous set of tools to get started with vision systems. However, some v4l2 camera's are not recognized as their drivers are `incomplete`.
This project provides a generic implementation of a v4l2 camera with basic streaming capabilities. Reading from the camera object returns an cv::Mat object.

### Adding your own camera?

A new camera is readily added. The only thing to take care of is to format the capturing, i.e. the pixel format. Depending on the latter, one should implement a conversion from that format to the standard opencv BGR color space.

### Interesting references

* Capturing a webcam using v4l2: https://jwhsmith.net/2014/12/capturing-a-webcam-stream-using-v4l2/
	A nice tutorial on basic v4l2 usage
* Tutorial slides: https://www.linuxtv.org/downloads/presentations/summit_jun_2010/20100206-fosdem.pdf
	Clear V4L2 tutorial slides with code examples.
* Vidcopy: https://github.com/lhelontra/vidcopy
	Copies a video to a virtual video device. The code is well written and a good follow-up to the previous tutorial
