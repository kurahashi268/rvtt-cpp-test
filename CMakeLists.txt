cmake_minimum_required(VERSION 3.15)
project(rvtt-cpp LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pthread")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Find required packages
find_package(Threads REQUIRED)

# PortAudio for audio capture
find_library(PORTAUDIO_LIBRARY NAMES portaudio libportaudio REQUIRED)
find_path(PORTAUDIO_INCLUDE_DIR portaudio.h REQUIRED)

# Whisper.cpp integration
set(WHISPER_CPP_DIR ${CMAKE_SOURCE_DIR}/core)

if(NOT EXISTS ${WHISPER_CPP_DIR})
    message(FATAL_ERROR "whisper.cpp core library not found at ${WHISPER_CPP_DIR}")
endif()

# Build whisper.cpp
add_subdirectory(${WHISPER_CPP_DIR} EXCLUDE_FROM_ALL)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${PORTAUDIO_INCLUDE_DIR}
    ${WHISPER_CPP_DIR}
)

# Source files
set(SOURCES
    src/main.cpp
    src/audio_capture.cpp
    src/transcriber.cpp
)

# Create executable
add_executable(rvtt ${SOURCES})

# Link libraries
target_link_libraries(rvtt
    whisper
    ${PORTAUDIO_LIBRARY}
    Threads::Threads
)

# Installation
install(TARGETS rvtt DESTINATION bin)

