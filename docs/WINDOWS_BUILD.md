# Building for Windows - Smallest Executable

This guide explains how to build the smallest possible Windows executable for RVTT-CPP using LLVM-MinGW.

## Prerequisites

### Option 1: Cross-Compile from Linux/WSL

1. **Install LLVM-MinGW**
   ```bash
   # Download latest release
   wget https://github.com/mstorsjo/llvm-mingw/releases/download/20240917/llvm-mingw-20240917-ucrt-ubuntu-20.04-x86_64.tar.xz
   
   # Extract
   tar -xf llvm-mingw-20240917-ucrt-ubuntu-20.04-x86_64.tar.xz
   
   # Set environment variable
   export LLVM_MINGW_ROOT=$PWD/llvm-mingw-20240917-ucrt-ubuntu-20.04-x86_64
   
   # Add to ~/.bashrc for permanent setup
   echo "export LLVM_MINGW_ROOT=$PWD/llvm-mingw-20240917-ucrt-ubuntu-20.04-x86_64" >> ~/.bashrc
   ```

2. **Install Ninja**
   ```bash
   sudo apt install ninja-build cmake
   ```

3. **Install PortAudio (for cross-compilation)**
   ```bash
   # You'll need to cross-compile PortAudio or use pre-built binaries
   # See: http://www.portaudio.com/
   ```

### Option 2: Native Build on Windows

1. **Install LLVM-MinGW**
   - Download from: https://github.com/mstorsjo/llvm-mingw/releases
   - Extract to `C:\llvm-mingw` (or your preferred location)
   - Add `C:\llvm-mingw\bin` to your PATH

2. **Install Ninja**
   ```powershell
   # Using winget
   winget install Ninja-build.Ninja
   
   # Or download from: https://github.com/ninja-build/ninja/releases
   ```

3. **Install CMake**
   ```powershell
   winget install Kitware.CMake
   ```

4. **Install PortAudio**
   - Download pre-built binaries or build from source
   - Make sure to build static library (`.a`)

## Build Configuration

The build is configured for maximum size optimization:

| Setting | Value | Purpose |
|---------|-------|---------|
| **Compiler** | `clang++` (LLVM-MinGW) | Better optimization than GCC |
| **Build Type** | `MinSizeRel` | Optimize for size |
| **Optimization** | `-O3` | Maximum optimization |
| **LTO** | `ON` | Link-time optimization |
| **Sections** | `-ffunction-sections -fdata-sections` | Split code into sections |
| **GC Sections** | `-Wl,--gc-sections` | Remove unused code |
| **Static** | `-static` | No DLL dependencies |
| **Strip** | `-s` / `llvm-strip` | Remove debug symbols |

## Building

### Cross-Compile from Linux

```bash
# Make sure LLVM_MINGW_ROOT is set
export LLVM_MINGW_ROOT=/path/to/llvm-mingw

# Build using the script
./build-windows.sh

# Or manually with CMake
cmake --preset windows-x64-release
cmake --build --preset windows-x64-release
```

### Native Build on Windows

```batch
REM Run from Command Prompt
build-windows.bat

REM Or manually with CMake
cmake --preset windows-x64-native
cmake --build --preset windows-x64-native
```

### Using VS Code

1. Open the project in VS Code
2. Install the "CMake Tools" extension
3. Press `Ctrl+Shift+P` and select "CMake: Select Configure Preset"
4. Choose `windows-x64-native` (on Windows) or `windows-x64-release` (cross-compile)
5. Press `F7` to build

## Further Size Reduction

### 1. UPX Compression

UPX can compress the executable even further (50-70% size reduction):

```bash
# Linux/WSL
sudo apt install upx-ucl
upx --best --lzma build-windows/rvtt.exe

# Windows
# Download from: https://upx.github.io/
upx --best --lzma build-windows\rvtt.exe
```

**Note:** Some antivirus software may flag UPX-compressed executables as suspicious.

### 2. Optimize Dependencies

- Build PortAudio as static library with size optimizations
- Disable unused Whisper.cpp features in `core/CMakeLists.txt`
- Remove unused audio backends from PortAudio

### 3. Strip More Aggressively

```bash
# Remove all symbols
llvm-strip --strip-all build-windows/rvtt.exe

# Remove debug sections
llvm-strip --strip-debug build-windows/rvtt.exe
```

## Architecture-Specific Builds

### x86-64 (Generic, Best Compatibility)
```bash
# Default - works on all x64 processors
cmake --preset windows-x64-release
```

### Native Architecture (Smallest, Less Portable)
To enable `-march=native` for your specific CPU, edit `cmake/llvm-mingw-toolchain.cmake`:

```cmake
# Change this line:
set(OPTIMIZATION_FLAGS "-O3 -march=x86-64 -mtune=generic ...")

# To this (if building on target machine):
set(OPTIMIZATION_FLAGS "-O3 -march=native -mtune=native ...")
```

**Warning:** `-march=native` binaries may not work on older CPUs!

## Build Output

The executable will be created at:
- **Linux/WSL:** `build-windows/rvtt.exe`
- **Windows:** `build-windows\rvtt.exe`

## Testing

To test the Windows executable:

```batch
cd build-windows
rvtt.exe ..\models\ggml-base.bin ja
```

## Troubleshooting

### "LLVM_MINGW_ROOT not set"
Make sure you've exported the environment variable:
```bash
export LLVM_MINGW_ROOT=/path/to/llvm-mingw
```

### "PortAudio not found"
You need to install or cross-compile PortAudio. For cross-compilation, you may need to set:
```bash
export PKG_CONFIG_PATH=/path/to/portaudio/lib/pkgconfig
```

### "Ninja not found"
Install Ninja:
- Linux: `sudo apt install ninja-build`
- Windows: `winget install Ninja-build.Ninja`

### Large executable size
1. Make sure you're building with `MinSizeRel` configuration
2. Check that LTO is enabled
3. Run `llvm-strip -s` on the executable
4. Consider using UPX compression

### Missing DLLs on Windows
Make sure static linking is enabled in the toolchain file. The executable should not require any DLLs except system ones (kernel32.dll, etc.).

## Expected File Size

Without dependencies optimization:
- **Before strip:** ~15-20 MB
- **After strip:** ~5-10 MB
- **After UPX:** ~2-4 MB

With full optimization (static PortAudio, minimal features):
- **After strip:** ~3-5 MB
- **After UPX:** ~1-2 MB

## Additional Resources

- LLVM-MinGW: https://github.com/mstorsjo/llvm-mingw
- Ninja: https://ninja-build.org/
- UPX: https://upx.github.io/
- PortAudio: http://www.portaudio.com/
- Whisper.cpp: https://github.com/ggerganov/whisper.cpp

