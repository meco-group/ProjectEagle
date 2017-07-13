# Useful examples
The default settings for the examples are found under include/examples_config.h . These allow the user to easily change the camera type, camera port, network interface, ... in order to configure the examples in a consistent way. Make sure these settings correspond to your system. Otherwise, segfaults will occur.
By default, examples are required to be run from the build directory in order to have the right relative paths. So, running Background is done from build, as: ./bin/Background

## 1. Snapshot
Take a snapshot and save the image.
## 2. Viewer
Open the video stream in a window.
## 3. AnalyzeFPS
Estimate the framerate for the camera.
## 4. AnalyzeCompression
* Estimate the framerate with compression.
* Estimate the ratio of compression time and capturing time.
## 5. ImageTransmitter - ImageReceiver
Transmit images for as long as a receiver is listening. The ImageTransmitter-ImageReceiver pair should be run together as a video transmission demo. When the execution stops, a report is shown about the amount of video data which has been transmitted over the network.
## 6. Background
Takes a few snapshots and averages these to get a nice background, necessary for the detection of the robots.
## 7. Detector
Detect robots in the viewing area. Unless your camera is really distorted, this demo will just work fine, no calibration needed. Codes being detected in the demo are 0, 1 and 9. A Background should be available (cfr. Background).
## 8. EagleTransmitter - EagleReceiver
Transmit images, robot and obstacle data. This should be run together with the receiver as a full demo. This program can be used as a starting point to everyone who wants to do detection of his own robots.

