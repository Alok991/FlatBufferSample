cmake_minimum_required(VERSION 3.12)

project(FlatBufferSample)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)

# Add subdirectory for FlatBuffers (assuming it's in a 'flatbuffers' directory)
add_subdirectory(flatbuffers)

# Define your project's source files and include directories
set(SOURCES
    src/main.cpp
    # Add other source files here
)

set(INCLUDE_DIRS
    include
    # Add other include directories here
)

# Create an executable for your project
add_executable(FlatBufferSample ${SOURCES})

# Include directories for your project
target_include_directories(FlatBufferSample PRIVATE ${INCLUDE_DIRS})

# Link FlatBuffers library to your project
target_link_libraries(FlatBufferSample PRIVATE flatbuffers)

# Optionally, you can set the output directory for the binary
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

