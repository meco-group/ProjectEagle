cmake_minimum_required(VERSION 2.8)
project( OPIcam )
find_package( OpenCV REQUIRED )
add_compile_options(-std=c++11)

set (SOURCE_FILES src/camera_interface.cpp src/v4l2_camera.cpp src/latitude_camera.cpp src/opi_camera.cpp src/see3_camera.cpp src/streamer.cpp)
set (HEADER_FILES include/camera_interface.h include/v4l2_camera.h include/latitude_camera.h include/opi_camera.h include/see3_camera.h include/streamer.h)
include_directories( include )

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable( LatitudeViewer examples/latitude_test.cpp ${SOURCE_FILES} ${HEADER_FILES} )
target_link_libraries( LatitudeViewer ${OpenCV_LIBS} )

add_executable( See3Viewer examples/see3cam_test.cpp ${SOURCE_FILES} ${HEADER_FILES} )
target_link_libraries( See3Viewer ${OpenCV_LIBS} )

add_executable( See3FPS examples/see3cam_test_fps.cpp ${SOURCE_FILES} ${HEADER_FILES} )
target_link_libraries( See3FPS ${OpenCV_LIBS} )

add_executable( OPIcapture examples/opi_test_capture.cpp ${SOURCE_FILES} ${HEADER_FILES} )
target_link_libraries( OPIcapture ${OpenCV_LIBS} )

add_executable( OPIfps examples/opi_test_fps.cpp ${SOURCE_FILES} ${HEADER_FILES} )
target_link_libraries( OPIfps ${OpenCV_LIBS} )

add_executable( OPIstream examples/opi_test_stream.cpp ${SOURCE_FILES} ${HEADER_FILES} )
target_link_libraries( OPIstream ${OpenCV_LIBS} )
