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
add_executable(GroundCalibrate calibrate_ground.cpp)
add_executable(DetectorTest detector_test.cpp)
add_executable(Fuse fuse.cpp)

set(LIBS ${LIBRARIES} Communicator Camera Detector)

# Define the libraries this project depends upon
target_link_libraries(TestCommunicator ${LIBS})
target_link_libraries(Calibrate ${LIBS})
target_link_libraries(StereoCalibrate ${LIBS})
target_link_libraries(Viewer ${LIBS})
target_link_libraries(ImageReceiver ${LIBS})
target_link_libraries(ImageTransmitter ${LIBS})
target_link_libraries(Snapshot ${LIBS})
target_link_libraries(EagleTransmitter ${LIBS})
target_link_libraries(Background ${LIBS})
target_link_libraries(GroundCalibrate ${LIBS})
target_link_libraries(DetectorTest ${LIBS})
target_link_libraries(Fuse ${LIBS})
