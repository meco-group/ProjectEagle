# Look for all source files in src and add them to the library
file(GLOB_RECURSE SOURCE_FILES *.cpp)

# Define an executable
add_executable(TestCommunicator test_communicator.cpp)
add_executable(Calibrate calibrate.cpp)
add_executable(StereoCalibrate stereo_calibrate.cpp)
add_executable(Viewer viewer.cpp)
add_executable(ImageReceiver image_receiver.cpp)
add_executable(ImageTransmitter image_transmitter.cpp)
add_executable(Snapshot snapshot.cpp)
add_executable(EagleTransmitter eagle_transmitter.cpp)
add_executable(Background background.cpp)

# Define the libraries this project depends upon
target_link_libraries(TestCommunicator Communicator)
target_link_libraries(Calibrate Calibrator Camera)
target_link_libraries(StereoCalibrate Calibrator Camera)
target_link_libraries(Viewer Camera Utils)
target_link_libraries(ImageReceiver Communicator Utils Detector)
target_link_libraries(ImageTransmitter Camera Communicator Utils Detector)
target_link_libraries(Snapshot Camera Communicator Utils)
target_link_libraries(EagleTransmitter Camera Communicator Utils Detector)
target_link_libraries(Background Camera)
