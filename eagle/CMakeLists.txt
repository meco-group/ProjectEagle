cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(eagle LANGUAGES CXX)

# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
include(GNUInstallDirs)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_definitions(-DSWM_ZYRE_CONFIG_DIR="${CMAKE_HOME_DIRECTORY}")
add_definitions(-std=c++11)

find_package(OpenCV REQUIRED)
find_package(ZMQ REQUIRED)
find_package(CZMQ REQUIRED)
find_package(ZYRE REQUIRED)

set(LIBRARIES   ${OpenCV_LIBRARIES}
                ${ZMQ_LIBRARIES}
                ${CZMQ_LIBRARIES}
                ${ZYRE_LIBRARIES}
    )

include_directories(src/utils
                    ${OpenCV_INCLUDE_DIRS}
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
add_subdirectory(src/communicator)
add_subdirectory(src/detector)

# Generate main header and install all headers
file(GLOB_RECURSE HEADER_FILES *.hpp *.h)
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
foreach(f ${HEADER_FILES})
    file(RELATIVE_PATH fr "${SRC_DIR}" ${f})
    if(NOT "${fr}" STREQUAL "eagle.h")
        get_filename_component(HEADER_PATH ${f} PATH)
        string(REPLACE ${SRC_DIR} "" HEADER_PATH ${HEADER_PATH})
        install(FILES ${f} DESTINATION "include/${PROJECT_NAME}/${HEADER_PATH}")
        list(APPEND incls "#include \"${fr}\"\n")
    endif()
endforeach()
file(WRITE src/eagle.h ${incls})
install(FILES src/eagle.h DESTINATION include/${PROJECT_NAME})

# Generate and install main project library
add_library(${PROJECT_NAME} SHARED $<TARGET_OBJECTS:camera>
                            $<TARGET_OBJECTS:communicator>
                            $<TARGET_OBJECTS:detector>
            )

target_include_directories( ${PROJECT_NAME} PUBLIC
                            $<BUILD_INTERFACE:${SRC_DIR}>
                            $<INSTALL_INTERFACE:include>/${PROJECT_NAME}/)
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

# If we have compiler requirements for this library, list them here
target_compile_features(${PROJECT_NAME} PUBLIC cxx_auto_type PRIVATE cxx_variadic_templates)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets DESTINATION lib/eagle)
install(EXPORT ${PROJECT_NAME}-targets DESTINATION lib/eagle)

# Generate and install package config file
set(CONF_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME}")
set(CONF_LIBRARY_DIRS "${CMAKE_INSTALL_PREFIX}/lib/${PROJECT_NAME}")
configure_file(cmake/${PROJECT_NAME}-config.cmake.in "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}-config.cmake" @ONLY)
install(FILES "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}-config.cmake" DESTINATION lib/${PROJECT_NAME})
