# Generic V4L2 Camera Drivers

## What is it?

OpenCV provides a marvelous set of tools to get started with vision systems. However, some v4l2 camera's are not recognized as their drivers are `incomplete`.
This project provides a generic implementation of a v4l2 camera with basic streaming capabilities. Reading from the camera object returns an cv::Mat object.

## Adding your own camera?

A new camera is readily added. The only thing to take care of is to format the capturing, i.e. the pixel format. Depending on the latter, one should implement a conversion from that format to the standard opencv BGR color space.

## Interesting references

* Capturing a webcam using v4l2: https://jwhsmith.net/2014/12/capturing-a-webcam-stream-using-v4l2/
	A nice tutorial on basic v4l2 usage
* Tutorial slides: https://www.linuxtv.org/downloads/presentations/summit_jun_2010/20100206-fosdem.pdf
	Clear V4L2 tutorial slides with code examples.
* Vidcopy: https://github.com/lhelontra/vidcopy
	Copies a video to a virtual video device. The code is well written and a good follow-up to the previous tutorial
