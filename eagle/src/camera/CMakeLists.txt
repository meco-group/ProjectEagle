project(camera LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

# Look for all source files in src and add them to the library
file(GLOB_RECURSE SOURCE_FILES *.cpp)
add_library(${PROJECT_NAME} OBJECT ${SOURCE_FILES})

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>)

# If we have compiler requirements for this library, list them here
target_compile_features(${PROJECT_NAME}
    PUBLIC cxx_auto_type
    PRIVATE cxx_variadic_templates)
