# Look for all source files in src and add them to the library
file(GLOB_RECURSE SOURCE_FILES *.cpp)

# Define an executable
add_executable(Viewer ${SOURCE_FILES})

# Define the libraries this project depends upon
target_link_libraries(Viewer
        LibCam
        LibIO)