project(LibCom LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

# Look for all source files in src and add them to the library
file(GLOB_RECURSE SOURCE_FILES *.cpp)
add_library(LibCom ${SOURCE_FILES})

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(LibCom PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE src)

# If we have compiler requirements for this library, list them
# here
target_compile_features(LibCom
        PUBLIC cxx_auto_type
        PRIVATE cxx_variadic_templates)

# Depend on a library that we defined in the top-level file
target_link_libraries(LibCom
        boost
        ${LIBRARIES}
        LibConfig)

# 'make install' to the correct locations (provided by GNUInstallDirs).
install(TARGETS LibCom EXPORT LibComConfig
        ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})  # This is for Windows
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into 'cmake'.
install(EXPORT LibComConfig DESTINATION share/LibComConfig/cmake)

# Every library has unit tests, of course
# Look for all source files in test and add them to the test executable
#file(GLOB_RECURSE TEST_SOURCE_FILES test/*.cpp)
#
## Create the executable
#add_executable(TestCam
#        ${TEST_SOURCE_FILES})
#
#target_link_libraries(TestCam
#        LibCom)
#
#add_test(TestCam TestCam)