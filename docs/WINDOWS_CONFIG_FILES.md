# Windows Build Configuration Files Summary

This document lists all configuration files created for building the smallest Windows executable.

## Created Files

### Build Scripts

1. **`build-windows.sh`**
   - Cross-compilation script for Linux/WSL
   - Automatically detects LLVM-MinGW
   - Builds with size optimizations
   - Optional UPX compression
   - Usage: `./build-windows.sh [clean]`

2. **`build-windows.bat`**
   - Native Windows build script
   - Uses LLVM-MinGW clang++
   - Interactive prompts for optimization options
   - Usage: `build-windows.bat [clean]`

### CMake Configuration

3. **`CMakePresets.json`**
   - CMake presets for Windows builds
   - Two presets:
     - `windows-x64-release` - Cross-compilation
     - `windows-x64-native` - Native Windows build
   - Configures: Ninja, MinSizeRel, static linking, LTO
   - Usage: `cmake --preset windows-x64-release`

4. **`cmake/llvm-mingw-toolchain.cmake`**
   - LLVM-MinGW cross-compilation toolchain file
   - Sets compilers (clang, clang++, llvm-ar, etc.)
   - Applies optimization flags:
     - `-O3 -flto=thin -ffunction-sections -fdata-sections`
     - `-static -Wl,--gc-sections -Wl,--strip-all`
   - Requires: `LLVM_MINGW_ROOT` environment variable

5. **`cmake/FindPortAudio-Windows.cmake`**
   - Helper module to find PortAudio on Windows
   - Prefers static libraries
   - Searches standard and custom paths
   - Creates imported target: `PortAudio::PortAudio`

### Modified Files

6. **`CMakeLists.txt`** (modified)
   - Added Windows-specific optimization flags
   - Platform detection (WIN32)
   - Conditional compilation for LLVM-MinGW:
     - LTO enabled
     - Function/data sections
     - Static linking
     - Strip flags
   - Maintains Linux/macOS compatibility

7. **`.gitignore`** (modified)
   - Excludes `build-windows/` directory
   - Keeps `.vscode/settings.json` and `.vscode/cmake-kits.json` in repo
   - Excludes auto-generated VS Code files

### VS Code Configuration

8. **`.vscode/settings.json`**
   - CMake Tools configuration
   - Uses CMake presets
   - Default preset: `windows-x64-native`
   - IntelliSense configuration
   - File watchers exclusion

9. **`.vscode/cmake-kits.json`**
   - CMake Kits for Windows builds
   - Two kits:
     - LLVM-MinGW Cross-Compile
     - LLVM-MinGW Native (Windows)

10. **`.vscode/extensions.json`**
    - Recommended VS Code extensions
    - CMake Tools, C++, Clangd

### Documentation

11. **`WINDOWS_BUILD.md`**
    - Comprehensive Windows build guide
    - Prerequisites and installation
    - Build instructions (cross-compile and native)
    - Size optimization techniques
    - Architecture-specific builds
    - Troubleshooting section

12. **`PORTAUDIO_WINDOWS.md`**
    - PortAudio build guide for Windows
    - Cross-compilation instructions
    - Native build instructions
    - Optimization flags for smallest library
    - Audio backend selection
    - Integration with RVTT-CPP

13. **`QUICKSTART_WINDOWS.md`**
    - Quick start guide (5-minute setup)
    - Step-by-step instructions
    - Two methods: cross-compile and native
    - Troubleshooting common issues
    - Expected results and file sizes
    - One-liner build commands

14. **`WINDOWS_CONFIG_FILES.md`** (this file)
    - Summary of all configuration files
    - Usage reference
    - Quick lookup table

## Quick Reference

### Environment Variables

| Variable | Purpose | Required For |
|----------|---------|--------------|
| `LLVM_MINGW_ROOT` | LLVM-MinGW installation path | Cross-compilation |
| `PORTAUDIO_ROOT` | PortAudio installation path | Both (if custom location) |

### Build Commands

| Command | Purpose |
|---------|---------|
| `cmake --preset windows-x64-release` | Configure (cross-compile) |
| `cmake --preset windows-x64-native` | Configure (native) |
| `cmake --build --preset windows-x64-release` | Build (cross-compile) |
| `cmake --build --preset windows-x64-native` | Build (native) |
| `./build-windows.sh` | Auto-build (Linux/WSL) |
| `build-windows.bat` | Auto-build (Windows) |

### Optimization Flags Applied

| Flag | File | Purpose |
|------|------|---------|
| `-O3` | toolchain.cmake | Maximum optimization |
| `-flto=thin` | toolchain.cmake | Link-time optimization |
| `-ffunction-sections` | toolchain.cmake, CMakeLists.txt | Section splitting |
| `-fdata-sections` | toolchain.cmake, CMakeLists.txt | Section splitting |
| `-Wl,--gc-sections` | toolchain.cmake, CMakeLists.txt | Remove dead code |
| `-Wl,--strip-all` | toolchain.cmake | Strip symbols |
| `-static` | toolchain.cmake, CMakeLists.txt | Static linking |
| `-static-libgcc` | toolchain.cmake | Static libgcc |
| `-static-libstdc++` | toolchain.cmake | Static libstdc++ |

## File Locations

```
rvtt-cpp/
├── build-windows.sh              # Linux build script
├── build-windows.bat             # Windows build script
├── CMakePresets.json             # CMake presets
├── CMakeLists.txt                # Modified main CMake file
├── .gitignore                    # Modified ignore rules
│
├── cmake/                        # CMake modules
│   ├── llvm-mingw-toolchain.cmake      # Toolchain file
│   └── FindPortAudio-Windows.cmake     # PortAudio finder
│
├── .vscode/                      # VS Code configuration
│   ├── settings.json             # Editor settings
│   ├── cmake-kits.json           # CMake kits
│   └── extensions.json           # Extension recommendations
│
└── docs/
    ├── WINDOWS_BUILD.md          # Full build guide
    ├── PORTAUDIO_WINDOWS.md      # PortAudio guide
    ├── QUICKSTART_WINDOWS.md     # Quick start
    └── WINDOWS_CONFIG_FILES.md   # This file
```

## Configuration Flow

```
1. User sets LLVM_MINGW_ROOT
   └─> cmake/llvm-mingw-toolchain.cmake
       └─> Sets compilers and flags

2. User runs: cmake --preset windows-x64-release
   └─> CMakePresets.json
       └─> References toolchain file
       └─> Sets build options (static, LTO, etc.)

3. CMake processes CMakeLists.txt
   └─> Detects Windows build (WIN32)
   └─> Applies platform-specific flags
   └─> Finds PortAudio (via FindPortAudio-Windows.cmake)

4. User runs: cmake --build --preset windows-x64-release
   └─> Ninja builds with all optimizations
   └─> Output: build-windows/rvtt.exe
```

## Customization

### Change Architecture
Edit `cmake/llvm-mingw-toolchain.cmake`:
```cmake
set(OPTIMIZATION_FLAGS "-O3 -march=x86-64 ...")  # Generic
# or
set(OPTIMIZATION_FLAGS "-O3 -march=native ...")  # Native (smaller, less portable)
```

### Change Optimization Level
Edit `CMakePresets.json`:
```json
"CMAKE_BUILD_TYPE": "MinSizeRel"  # Smallest
// or
"CMAKE_BUILD_TYPE": "Release"     # Fastest
```

### Disable LTO
Edit `CMakePresets.json`:
```json
"GGML_LTO": "OFF"
```

### Add Custom Flags
Edit `cmake/llvm-mingw-toolchain.cmake`:
```cmake
set(OPTIMIZATION_FLAGS "${OPTIMIZATION_FLAGS} -your-custom-flag")
```

## Testing Configuration

### Verify Optimization Flags
```bash
# Check compile commands
cat build-windows/compile_commands.json | grep -o '\-O[0-9]'

# Check LTO is enabled
grep CMAKE_INTERPROCEDURAL_OPTIMIZATION build-windows/CMakeCache.txt
```

### Verify Static Linking
```bash
# On Linux (with LLVM-MinGW)
x86_64-w64-mingw32-objdump -p build-windows/rvtt.exe | grep "DLL Name"

# On Windows
dumpbin /dependents build-windows\rvtt.exe
```

Should only show system DLLs (KERNEL32.dll, etc.).

### Check Binary Size
```bash
ls -lh build-windows/rvtt.exe
```

Expected: 5-10 MB (before UPX), 2-4 MB (after UPX)

## Support Matrix

| Platform | Build | Status |
|----------|-------|--------|
| Linux → Windows | ✓ Cross-compile | Tested |
| Windows → Windows | ✓ Native | Tested |
| WSL → Windows | ✓ Cross-compile | Tested |
| macOS → Windows | ⚠️ Cross-compile | Should work (untested) |

## Troubleshooting Quick Links

- LLVM-MinGW issues → `WINDOWS_BUILD.md#troubleshooting`
- PortAudio issues → `PORTAUDIO_WINDOWS.md#troubleshooting`
- Build errors → `QUICKSTART_WINDOWS.md#troubleshooting`
- Size issues → `WINDOWS_BUILD.md#further-size-reduction`

## Version Information

These configuration files are compatible with:
- CMake 3.23+
- LLVM-MinGW 20240917+
- Ninja 1.10+
- PortAudio 19.7.0+
- Whisper.cpp (latest)

## Next Steps

1. **First time setup:**
   - Read `QUICKSTART_WINDOWS.md`
   - Install prerequisites
   - Run build script

2. **Customization:**
   - Modify `cmake/llvm-mingw-toolchain.cmake` for flags
   - Edit `CMakePresets.json` for build options
   - Update `CMakeLists.txt` for features

3. **Distribution:**
   - Test `rvtt.exe` on target Windows systems
   - Package with model file
   - Optionally sign executable

## License

These configuration files are part of RVTT-CPP project.

