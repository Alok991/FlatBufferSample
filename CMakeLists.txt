cmake_minimum_required(VERSION 3.12)

project(fb_encoder)

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
add_executable(fb_encoder ${SOURCES})

# Include directories for your project
target_include_directories(fb_encoder PRIVATE ${INCLUDE_DIRS})

# Link FlatBuffers library to your project
target_link_libraries(fb_encoder PRIVATE flatbuffers)

# Custom command to generate C++ code from FlatBuffer schema files
set(FLATBUFFERS_SCHEMA_DIR ${CMAKE_SOURCE_DIR}/flatbuffers_schemas)
set(FLATBUFFERS_GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)

message("${FLATBUFFERS_GENERATED_DIR}")

add_custom_command(
    OUTPUT ${FLATBUFFERS_GENERATED_DIR}/client_generated.h
    COMMAND flatc --cpp -o ${FLATBUFFERS_GENERATED_DIR} ${FLATBUFFERS_SCHEMA_DIR}/client.fbs
    DEPENDS ${FLATBUFFERS_SCHEMA_DIR}/client.fbs
)

# Add the generated files to your project's include directories
target_include_directories(fb_encoder PRIVATE ${FLATBUFFERS_GENERATED_DIR})

# Add the generated header as a dependency for your project
target_sources(fb_encoder PRIVATE ${FLATBUFFERS_GENERATED_DIR}/client_generated.h)

#get_target_property(fb_encoder_include_dirs fb_encoder INCLUDE_DIRECTORIES)
#message("${fb_encoder_include_dirs}")