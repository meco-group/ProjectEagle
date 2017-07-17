# Look for all source files in src and add them to the library
file(GLOB_RECURSE SOURCE_FILES *.cpp)

# Define an executable
add_executable(ImageTransmitter src/image_transmitter.cpp)
add_executable(ImageReceiver src/image_receiver.cpp)

# Define the libraries this project depends upon
target_link_libraries(ImageTransmitter
        LibCam
        LibCom)
target_link_libraries(ImageReceiver
        LibCom
        LibIO)