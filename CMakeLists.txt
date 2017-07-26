cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(ProjectEagle)

# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
include(GNUInstallDirs)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 11)

ADD_DEFINITIONS(-DSWM_ZYRE_CONFIG_DIR="${CMAKE_HOME_DIRECTORY}")
ADD_DEFINITIONS(-std=c++11)

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

include_directories(src/include
        ${ZMQ_INCLUDE_DIRS}
        ${ZMQ_INCLUDE_DIRS}
        ${CZMQ_INCLUDE_DIRS}
        ${ZYRE_INCLUDE_DIRS}
        /usr/include/python2.7
        )

# Include Boost as an imported target
find_package(Boost REQUIRED)
add_library(boost INTERFACE IMPORTED)
set_property(TARGET boost PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Targets that we develop here
enable_testing()
add_subdirectory(src/libcom)
add_subdirectory(src/libcam)
add_subdirectory(src/libconfig)
add_subdirectory(src/libio)
add_subdirectory(src/viewer)
add_subdirectory(src/image_transmitter)
add_subdirectory(src/calibrator)
add_subdirectory(src/eagle)