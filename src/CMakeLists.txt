add_library(LibEagle camera_interface.cpp v4l2_camera.cpp latitude_camera.cpp opi_camera.cpp see3_camera.cpp streamer.cpp communicator.cpp robot.cpp obstacle.cpp detector.cpp)
target_link_libraries(LibEagle ${LIBRARIES})
