cmake_minimum_required(VERSION 3.15)
project(rvtt-cpp LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Platform detection
if(WIN32)
    set(WINDOWS_BUILD ON)
else()
    set(WINDOWS_BUILD OFF)
endif()

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND WINDOWS_BUILD)
    # LLVM-MinGW optimizations for smallest binary
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG -ffunction-sections -fdata-sections")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -O3 -DNDEBUG -ffunction-sections -fdata-sections")
    
    # Link-time optimization
    if(NOT DEFINED CMAKE_INTERPROCEDURAL_OPTIMIZATION)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    endif()
    
    # Linker flags for smallest binary
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections -Wl,--strip-all -Wl,-O3")
    
    # Static linking on Windows
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
else()
    # Standard flags for Linux/macOS
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pthread")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

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

# Windows-specific settings for smallest binary
if(WINDOWS_BUILD)
    # Enable LTO for this target
    set_target_properties(rvtt PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION ON
    )
    
    # Additional Windows libraries for static linking
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_link_libraries(rvtt
            -static
            winpthread
        )
    endif()
endif()

# Installation
install(TARGETS rvtt DESTINATION bin)

