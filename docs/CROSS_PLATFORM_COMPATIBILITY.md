# Cross-Platform Compatibility (Windows & Linux)

## Overview
This document describes the cross-platform compatibility features implemented in rvtt-cpp to ensure it runs smoothly on both Windows and Linux systems.

## Changes Made

### 1. Signal Handling (`src/main.cpp`)

#### Problem
The original code used Unix-specific signal handling (`SIGINT`, `SIGTERM`) which doesn't work properly on Windows.

#### Solution
Implemented platform-specific signal handling:

**Linux/Unix:**
- Uses standard POSIX signals: `SIGINT` and `SIGTERM`
- Signal handler function: `signalHandler(int signal)`

**Windows:**
- Uses Windows Console API: `SetConsoleCtrlHandler`
- Handles: `CTRL_C_EVENT`, `CTRL_BREAK_EVENT`, `CTRL_CLOSE_EVENT`
- Console handler function: `consoleHandler(DWORD signal)`

### 2. UTF-8 Console Output (Windows)

#### Problem
Windows console by default doesn't display UTF-8 characters (like emoji) correctly.

#### Solution
Added `SetConsoleOutputCP(CP_UTF8)` on Windows to enable proper UTF-8 display, ensuring the 🎤 emoji and other Unicode characters render correctly.

### 3. Platform Detection

The code uses preprocessor directives to detect the platform:
```cpp
#ifdef _WIN32
    // Windows-specific code
#else
    // Linux/Unix-specific code
#endif
```

## Files Modified

### `src/main.cpp`
- Added platform-specific includes:
  - Windows: `<windows.h>`
  - Linux: `<signal.h>`
- Implemented separate signal handlers for each platform
- Added UTF-8 console configuration for Windows
- Used conditional compilation for platform-specific initialization

## Cross-Platform Features Already Present

### Standard C++ Threading
All threading primitives use standard C++11/14/17 features:
- `std::thread`
- `std::mutex`
- `std::condition_variable`
- `std::atomic<bool>`

These work identically on both Windows and Linux.

### PortAudio
The audio capture system uses PortAudio, which is inherently cross-platform and provides a unified API for both Windows and Linux.

### CMake Build System
The `CMakeLists.txt` already has platform-specific configurations:
- Windows: Static linking, LLVM-MinGW optimizations
- Linux: Standard GCC/Clang flags with pthread support

## Testing

### Linux Build
```bash
./build.sh
./build/rvtt models/ggml-base.bin ja
```

### Windows Build
```bash
build-windows.bat
# or
./build-windows.sh
build/rvtt.exe models/ggml-base.bin ja
```

## Platform-Specific Notes

### Windows
- Uses static linking by default (no DLL dependencies except system DLLs)
- Console properly handles UTF-8 output
- Ctrl+C, Ctrl+Break, and console close events all trigger graceful shutdown

### Linux
- Uses dynamic linking for system libraries
- SIGINT (Ctrl+C) and SIGTERM trigger graceful shutdown
- Requires pthread support (automatically linked via CMake)

## Dependencies

Both platforms require:
1. **PortAudio**: Cross-platform audio I/O library
2. **Whisper.cpp**: Cross-platform speech recognition library
3. **C++17 compiler**: GCC/Clang on Linux, LLVM-MinGW on Windows

## Future Considerations

The codebase is now fully cross-platform. Additional platforms (macOS, BSD) should work with minimal or no changes due to the standard C++ and POSIX-like APIs used.


