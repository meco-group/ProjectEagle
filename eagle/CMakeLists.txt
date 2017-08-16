cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(ProjectEagle)

# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
include(GNUInstallDirs)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 11)

add_definitions(-DSWM_ZYRE_CONFIG_DIR="${CMAKE_HOME_DIRECTORY}")
add_definitions(-std=c++11)

find_package(OpenCV REQUIRED)
find_package(ZMQ REQUIRED)
find_package(CZMQ REQUIRED)
find_package(ZYRE REQUIRED)
find_package(Threads REQUIRED)

set(LIBRARIES
        ${OpenCV_LIBS}
        ${ZMQ_LIBRARIES}
        ${CZMQ_LIBRARIES}
        ${ZYRE_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        )

include_directories(src/utils
        ${ZMQ_INCLUDE_DIRS}
        ${ZMQ_INCLUDE_DIRS}
        ${CZMQ_INCLUDE_DIRS}
        ${ZYRE_INCLUDE_DIRS}
        )

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Targets that we develop here
add_subdirectory(src/exec)
add_subdirectory(src/camera)
add_subdirectory(src/calibrator)
add_subdirectory(src/communicator)
add_subdirectory(src/configurator)
add_subdirectory(src/detector)
