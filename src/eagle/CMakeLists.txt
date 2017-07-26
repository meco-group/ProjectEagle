# Look for all source files in src and add them to the library
file(GLOB_RECURSE SOURCE_FILES src/helpers/*.cpp)

# Define an executable
add_executable(EagleTransmitter src/eagle_transmitter.cpp ${SOURCE_FILES})
add_executable(Background src/background.cpp)

# Define the libraries this project depends upon
target_link_libraries(EagleTransmitter
        LibCam
        LibCom
        LibConfig
        LibIO)

# Define the libraries this project depends upon
target_link_libraries(Background
        LibCam)