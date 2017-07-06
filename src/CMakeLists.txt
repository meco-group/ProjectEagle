add_library(LibEagle cal/cal_settings.cpp cal/calibrator.cpp cam/camera_interface.cpp cam/v4l2_camera.cpp cam/latitude_camera.cpp cam/opi_camera.cpp cam/pi_camera.cpp cam/see3_camera.cpp comm/streamer.cpp comm/communicator.cpp robot.cpp obstacle.cpp detector.cpp cal/calibrator.cpp cal/board_settings.cpp)
target_link_libraries(LibEagle ${LIBRARIES})
