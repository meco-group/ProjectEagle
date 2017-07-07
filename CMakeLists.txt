cmake_minimum_required(VERSION 2.8)
project( ProjectEagle )

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

ADD_DEFINITIONS(-DSWM_ZYRE_CONFIG_DIR="${CMAKE_HOME_DIRECTORY}")
ADD_DEFINITIONS(-std=c++11)

find_package(OpenCV REQUIRED)
find_package(ZMQ REQUIRED)
find_package(CZMQ REQUIRED)
find_package(ZYRE REQUIRED)

set (LIBRARIES ${OpenCV_LIBS} ${ZMQ_LIBRARIES} ${CZMQ_LIBRARIES} ${ZYRE_LIBRARIES} LibEagle)
include_directories( include ${ZMQ_INCLUDE_DIRS} ${ZMQ_INCLUDE_DIRS} ${CZMQ_INCLUDE_DIRS}  ${ZYRE_INCLUDE_DIRS})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)

add_executable( Viewer examples/viewer.cpp )
add_executable( Snapshot examples/snapshot.cpp )
add_executable( ImageTransmitter examples/image_transmitter.cpp )
add_executable( ImageReceiver examples/image_receiver.cpp )
add_executable( Background examples/background.cpp)
add_executable( Detector examples/detector.cpp )
add_executable( EagleTransmitter examples/eagle_transmitter.cpp )
add_executable( EagleReceiver examples/eagle_receiver.cpp )

add_executable( AnalyzeFPS examples/tests/analyze_fps.cpp )
add_executable( AnalyzeCompression examples/tests/analyze_compression.cpp)
add_executable( Communicator examples/tests/communicator_test.cpp )
add_executable( SimpleTransmitter examples/tests/simple_transmitter.cpp )

add_executable( Experiments spike/experiments.cpp spike/cam_test.cpp spike/cal_test.cpp spike/detect_test.cpp)

add_executable( Calibrate examples/calibration.cpp )

target_link_libraries( Viewer ${LIBRARIES} )
target_link_libraries( Snapshot ${LIBRARIES} )
target_link_libraries( ImageTransmitter ${LIBRARIES})
target_link_libraries( ImageReceiver ${LIBRARIES})
target_link_libraries( Background ${LIBRARIES})
target_link_libraries( Detector ${LIBRARIES})
target_link_libraries( EagleTransmitter ${LIBRARIES})
target_link_libraries( EagleReceiver ${LIBRARIES})

target_link_libraries( AnalyzeFPS ${LIBRARIES} )
target_link_libraries( AnalyzeCompression ${LIBRARIES} )
target_link_libraries( Communicator ${LIBRARIES})
target_link_libraries( SimpleTransmitter ${LIBRARIES})

target_link_libraries( Experiments ${LIBRARIES})

target_link_libraries( Calibrate ${LIBRARIES})
