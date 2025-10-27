# Quick Start - Windows Build

Build the smallest Windows executable in 5 minutes (if you have dependencies ready).

## Prerequisites Checklist

- [ ] LLVM-MinGW installed
- [ ] Ninja build system
- [ ] CMake 3.23+
- [ ] PortAudio (static library)
- [ ] Whisper model file

## Method 1: Cross-Compile from Linux/WSL (Recommended)

### Step 1: Install LLVM-MinGW
```bash
wget https://github.com/mstorsjo/llvm-mingw/releases/download/20240917/llvm-mingw-20240917-ucrt-ubuntu-20.04-x86_64.tar.xz
tar -xf llvm-mingw-20240917-ucrt-ubuntu-20.04-x86_64.tar.xz
export LLVM_MINGW_ROOT=$PWD/llvm-mingw-20240917-ucrt-ubuntu-20.04-x86_64
```

### Step 2: Install Ninja
```bash
sudo apt install ninja-build
```

### Step 3: Build PortAudio (Optional - see PORTAUDIO_WINDOWS.md)
```bash
# If you have a pre-built static PortAudio, skip this
export PORTAUDIO_ROOT=/path/to/portaudio/install
```

### Step 4: Build RVTT
```bash
cd /path/to/rvtt-cpp
./build-windows.sh
```

### Step 5: Test
```bash
# Copy to Windows machine and run
cd build-windows
rvtt.exe ..\models\ggml-base.bin ja
```

## Method 2: Native Build on Windows

### Step 1: Install Tools
```powershell
# Install LLVM-MinGW (download and extract manually)
# Add to PATH: C:\llvm-mingw\bin

# Install Ninja
winget install Ninja-build.Ninja

# Install CMake
winget install Kitware.CMake
```

### Step 2: Build
```batch
cd C:\path\to\rvtt-cpp
build-windows.bat
```

## Expected Results

```
=== Build Successful! ===

Executable: build-windows/rvtt.exe
Size: 5-10 MB (before UPX)
Final size: 2-4 MB (after UPX)
```

## Optimization Flags Applied

Your build automatically uses:

| Flag | Purpose | Benefit |
|------|---------|---------|
| `-O3` | Maximum optimization | Speed + size |
| `-flto` | Link-time optimization | 10-30% smaller |
| `-ffunction-sections` | Split functions | Remove unused code |
| `-fdata-sections` | Split data | Remove unused data |
| `-Wl,--gc-sections` | Garbage collect | Remove dead code |
| `-static` | Static linking | No DLLs needed |
| `-s` | Strip symbols | 30-50% smaller |

## Troubleshooting

### Problem: "LLVM_MINGW_ROOT not set"
**Solution:**
```bash
export LLVM_MINGW_ROOT=/path/to/llvm-mingw
```

### Problem: "PortAudio not found"
**Solution:**
```bash
# Option 1: Set path
export PORTAUDIO_ROOT=/path/to/portaudio/install

# Option 2: Install system-wide (if cross-compiling)
# See PORTAUDIO_WINDOWS.md for details
```

### Problem: "Ninja not found"
**Solution:**
```bash
# Linux
sudo apt install ninja-build

# Windows
winget install Ninja-build.Ninja
```

### Problem: Executable is too large (>20 MB)
**Checks:**
1. Is it using static PortAudio? Check build output
2. Is LTO enabled? Check CMakeCache.txt for `CMAKE_INTERPROCEDURAL_OPTIMIZATION:BOOL=ON`
3. Is it stripped? Run: `llvm-strip -s build-windows/rvtt.exe`
4. Try UPX: `upx --best --lzma build-windows/rvtt.exe`

### Problem: Missing DLLs on Windows
**Solution:** Make sure static linking is working:
```bash
# Check DLL dependencies (on Windows)
dumpbin /dependents build-windows\rvtt.exe

# Or on Linux with LLVM-MinGW
x86_64-w64-mingw32-objdump -p build-windows/rvtt.exe | grep "DLL Name"
```

Should only show system DLLs (KERNEL32.dll, etc.), no PortAudio or libgcc/libstdc++.

## VS Code Setup (Optional)

1. Install extensions (auto-prompted when opening project)
2. Press `Ctrl+Shift+P` → "CMake: Select Configure Preset"
3. Choose `windows-x64-native` (Windows) or `windows-x64-release` (cross-compile)
4. Press `F7` to build

## Full Documentation

- **Windows Build Guide:** See `WINDOWS_BUILD.md`
- **PortAudio Setup:** See `PORTAUDIO_WINDOWS.md`
- **General Usage:** See `README.md`

## One-Liner Build (with dependencies ready)

### Linux
```bash
export LLVM_MINGW_ROOT=/path/to/llvm-mingw && ./build-windows.sh
```

### Windows
```batch
build-windows.bat
```

## Distribution

Your Windows executable is completely self-contained (no DLLs needed).

**Ship these files:**
- `rvtt.exe` - The application
- `models/ggml-base.bin` - Whisper model

**That's it!** No Visual C++ Redistributables, no PortAudio DLL, nothing else needed.

## Performance Notes

The optimized build will be:
- ✓ **Smaller** than debug builds (70-90% smaller)
- ✓ **Faster** than non-optimized builds (2-5x faster)
- ✓ **Portable** - runs on any x64 Windows (7+)
- ✓ **No dependencies** - single executable + model file

---

**Build time:** ~5-15 minutes (depending on hardware and dependencies)

