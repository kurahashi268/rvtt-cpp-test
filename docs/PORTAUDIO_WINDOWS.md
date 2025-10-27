# Building PortAudio for Windows (Static Library)

To build the smallest Windows executable, you need to compile PortAudio as a static library with LLVM-MinGW.

## Quick Start - Pre-built Binary

If you want to skip compilation, you can download pre-built PortAudio binaries:
- https://github.com/PortAudio/portaudio/releases

However, for the smallest binary, building from source with matching flags is recommended.

## Building PortAudio with LLVM-MinGW

### Prerequisites

1. LLVM-MinGW installed and `LLVM_MINGW_ROOT` set
2. CMake and Ninja

### Cross-Compile from Linux

```bash
# Download PortAudio
git clone https://github.com/PortAudio/portaudio.git
cd portaudio

# Create build directory
mkdir build-mingw && cd build-mingw

# Set toolchain
export LLVM_MINGW_ROOT=/path/to/llvm-mingw
export TOOLCHAIN=${LLVM_MINGW_ROOT}/bin/x86_64-w64-mingw32

# Configure with size optimizations
cmake .. \
    -G Ninja \
    -DCMAKE_SYSTEM_NAME=Windows \
    -DCMAKE_C_COMPILER=${TOOLCHAIN}-clang \
    -DCMAKE_CXX_COMPILER=${TOOLCHAIN}-clang++ \
    -DCMAKE_RC_COMPILER=${TOOLCHAIN}-windres \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DCMAKE_C_FLAGS="-O3 -flto -ffunction-sections -fdata-sections" \
    -DCMAKE_EXE_LINKER_FLAGS="-static -Wl,--gc-sections -flto" \
    -DPA_BUILD_SHARED=OFF \
    -DPA_BUILD_STATIC=ON \
    -DPA_USE_ASIO=OFF \
    -DPA_USE_DS=ON \
    -DPA_USE_WMME=ON \
    -DPA_USE_WASAPI=ON \
    -DPA_USE_WDMKS=OFF \
    -DCMAKE_INSTALL_PREFIX=$PWD/install

# Build
ninja

# Install to local directory
ninja install

# Set environment variable for RVTT-CPP build
export PORTAUDIO_ROOT=$PWD/install
```

### Native Build on Windows

```batch
REM Download PortAudio
git clone https://github.com/PortAudio/portaudio.git
cd portaudio

REM Create build directory
mkdir build && cd build

REM Configure
cmake .. ^
    -G Ninja ^
    -DCMAKE_C_COMPILER=clang ^
    -DCMAKE_CXX_COMPILER=clang++ ^
    -DCMAKE_BUILD_TYPE=MinSizeRel ^
    -DCMAKE_C_FLAGS="-O3 -flto -ffunction-sections -fdata-sections" ^
    -DCMAKE_EXE_LINKER_FLAGS="-static -Wl,--gc-sections -flto" ^
    -DPA_BUILD_SHARED=OFF ^
    -DPA_BUILD_STATIC=ON ^
    -DPA_USE_ASIO=OFF ^
    -DPA_USE_DS=ON ^
    -DPA_USE_WMME=ON ^
    -DPA_USE_WASAPI=ON ^
    -DPA_USE_WDMKS=OFF ^
    -DCMAKE_INSTALL_PREFIX=%CD%\install

REM Build
ninja

REM Install
ninja install

REM Set environment variable
set PORTAUDIO_ROOT=%CD%\install
```

## Using PortAudio with RVTT-CPP

After building PortAudio, set the environment variable:

### Linux/WSL
```bash
export PORTAUDIO_ROOT=/path/to/portaudio/build-mingw/install
```

### Windows
```batch
set PORTAUDIO_ROOT=C:\path\to\portaudio\build\install
```

Or modify `CMakePresets.json` to include PortAudio path:

```json
{
  "configurePresets": [
    {
      "name": "windows-x64-release",
      "cacheVariables": {
        "PORTAUDIO_ROOT": "/path/to/portaudio/install",
        ...
      }
    }
  ]
}
```

## Alternative: Modify RVTT-CPP CMakeLists.txt

You can also modify the main `CMakeLists.txt` to use the imported target:

```cmake
# After: find_library(PORTAUDIO_LIBRARY ...)
# Add:
if(PORTAUDIO_LIBRARY MATCHES "libportaudio\\.a$")
    message(STATUS "Using static PortAudio: ${PORTAUDIO_LIBRARY}")
else()
    message(WARNING "Not using static PortAudio. Binary will be larger.")
endif()
```

## Audio Backends

For minimum size, use only what you need:

| Backend | Purpose | Recommended |
|---------|---------|-------------|
| **WMME** | Windows Multimedia | ✓ Yes (basic, small) |
| **DirectSound** | DirectSound API | ✓ Yes (good compatibility) |
| **WASAPI** | Windows Audio Session API | ✓ Yes (modern, low latency) |
| **ASIO** | Professional audio | ✗ No (large, rarely needed) |
| **WDM-KS** | Kernel Streaming | ✗ No (complex, rarely needed) |

## Troubleshooting

### "Cannot find portaudio.h"
Set `PORTAUDIO_ROOT` to your PortAudio installation directory.

### "Undefined reference to PortAudio symbols"
Make sure you're linking against the static library (`libportaudio.a`), not the DLL import library.

### Large executable size
1. Rebuild PortAudio with size optimization flags
2. Disable unused audio backends
3. Use `-flto` (Link Time Optimization)

## Expected Impact on Binary Size

- **Without static PortAudio:** Need to ship `portaudio.dll` (~500 KB+)
- **With static PortAudio:** All code embedded (~200-400 KB added to exe)
- **Total savings:** Single executable, easier deployment

## Minimal PortAudio Configuration

For the absolute smallest build, use only WMME:

```bash
cmake .. \
    -DPA_USE_ASIO=OFF \
    -DPA_USE_DS=OFF \
    -DPA_USE_WMME=ON \
    -DPA_USE_WASAPI=OFF \
    -DPA_USE_WDMKS=OFF \
    ...
```

This reduces PortAudio size by ~50% but may have higher latency.

