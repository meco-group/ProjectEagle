cmake_minimum_required(VERSION 2.8)
project( ProjectEagle )

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

ADD_DEFINITIONS(-DSWM_ZYRE_CONFIG_DIR="${CMAKE_HOME_DIRECTORY}")
ADD_DEFINITIONS(-std=c++11)

find_package(OpenCV REQUIRED)
find_package(ZMQ REQUIRED)
find_package(CZMQ REQUIRED)
find_package(ZYRE REQUIRED)

set (SOURCE_FILES src/camera_interface.cpp src/v4l2_camera.cpp src/latitude_camera.cpp src/opi_camera.cpp src/streamer.cpp)
set (HEADER_FILES include/camera_interface.h include/v4l2_camera.h include/latitude_camera.h include/opi_camera.h include/streamer.h)
set (LIBRARIES ${OpenCV_LIBS} ${CZMQ_LIBRARIES} ${ZYRE_LIBRIES})

include_directories(
    include
    ${ZMQ_INCLUDE_DIRS}
    ${CZMQ_INCLUDE_DIRS}
    ${ZYRE_INCLUDE_DIRS}
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable( LatitudeViewer examples/latitude_test.cpp ${SOURCE_FILES} ${HEADER_FILES} )
add_executable( See3Viewer examples/see3cam_test.cpp ${SOURCE_FILES} ${HEADER_FILES} )
add_executable( See3FPS examples/see3cam_test_fps.cpp ${SOURCE_FILES} ${HEADER_FILES} )
add_executable( OPIcapture examples/opi_test_capture.cpp ${SOURCE_FILES} ${HEADER_FILES} )
add_executable( OPIfps examples/opi_test_fps.cpp ${SOURCE_FILES} ${HEADER_FILES} )
add_executable( OPIstream examples/opi_test_stream.cpp ${SOURCE_FILES} ${HEADER_FILES} )
target_link_libraries( LatitudeViewer ${LIBRARIES} )
target_link_libraries( See3Viewer ${LIBRARIES} )
target_link_libraries( See3FPS ${LIBRARIES} )
target_link_libraries( OPIcapture ${LIBRARIES} )
target_link_libraries( OPIfps ${LIBRARIES} )
target_link_libraries( OPIstream ${LIBRARIES} )
