# Windows Build - Getting Started

Welcome! This guide will help you build the smallest possible Windows executable for RVTT-CPP.

## 📚 Documentation Index

Choose the guide that fits your needs:

### 🚀 Quick Start (5 minutes)
**→ [QUICKSTART_WINDOWS.md](QUICKSTART_WINDOWS.md)**
- Prerequisites checklist
- Step-by-step build commands
- Troubleshooting common issues
- **Start here if you want to build quickly!**

### 📖 Complete Guide
**→ [WINDOWS_BUILD.md](WINDOWS_BUILD.md)**
- Detailed installation instructions
- Cross-compilation and native builds
- Size optimization techniques
- Architecture-specific builds
- Expected file sizes
- **Read this for comprehensive understanding**

### 🔧 PortAudio Setup
**→ [PORTAUDIO_WINDOWS.md](PORTAUDIO_WINDOWS.md)**
- Build PortAudio from source
- Cross-compilation instructions
- Static library configuration
- Audio backend selection
- **Only needed if you don't have PortAudio**

### 📋 Configuration Reference
**→ [WINDOWS_CONFIG_FILES.md](WINDOWS_CONFIG_FILES.md)**
- List of all configuration files
- File-by-file documentation
- Customization options
- Quick lookup table
- **For understanding the build system**

### 📊 Summary & Overview
**→ [BUILD_SUMMARY.md](BUILD_SUMMARY.md)**
- What's been configured
- Optimization flags explained
- Size comparisons
- Feature matrix
- **For a bird's-eye view**

### 📝 Quick Recipe Card
**→ [WINDOWS_BUILD_RECIPE.txt](WINDOWS_BUILD_RECIPE.txt)**
- One-page reference card
- Command cheat sheet
- Troubleshooting tips
- **Print this for quick reference!**

## 🎯 TL;DR - Build in 3 Commands

### Linux/WSL
```bash
export LLVM_MINGW_ROOT=/path/to/llvm-mingw
cd /path/to/rvtt-cpp
./build-windows.sh
```

### Windows
```batch
REM Add LLVM-MinGW to PATH first
cd C:\path\to\rvtt-cpp
build-windows.bat
```

## 🔑 Key Files

| File | Purpose |
|------|---------|
| `CMakePresets.json` | Build presets for CMake |
| `cmake/llvm-mingw-toolchain.cmake` | LLVM-MinGW cross-compilation toolchain |
| `build-windows.sh` | Linux/WSL build script |
| `build-windows.bat` | Windows build script |
| `.vscode/settings.json` | VS Code CMake configuration |

## 📦 What You Get

✅ **Single executable** - No DLL dependencies (except Windows system)  
✅ **Optimized size** - 5-10 MB (2-4 MB with UPX)  
✅ **Fast performance** - LTO and -O3 optimized  
✅ **Static linking** - No Visual C++ Runtime needed  
✅ **Portable** - Works on any x64 Windows 7+  

## 🛠️ Requirements

### For Cross-Compilation (Linux/WSL → Windows)
- LLVM-MinGW (https://github.com/mstorsjo/llvm-mingw/releases)
- Ninja build system
- CMake 3.23+
- PortAudio (static library)

### For Native Windows Build
- LLVM-MinGW for Windows
- Ninja build system
- CMake 3.23+
- PortAudio (static library)

## 🎓 Build Methods

We support three build methods:

### 1. Using Build Scripts (Easiest)
```bash
./build-windows.sh        # Linux/WSL
build-windows.bat         # Windows
```

### 2. Using CMake Presets
```bash
cmake --preset windows-x64-release
cmake --build --preset windows-x64-release
```

### 3. Using VS Code
- Install CMake Tools extension
- Select preset: `windows-x64-release` or `windows-x64-native`
- Press F7 to build

## 🔍 Optimization Flags

Your build automatically applies these flags for the smallest binary:

```cmake
# Compiler
-O3                    # Maximum optimization
-march=x86-64         # Generic x64 (or -march=native)
-flto=thin            # Link-time optimization
-ffunction-sections   # Split functions
-fdata-sections       # Split data

# Linker
-static               # Static linking
-Wl,--gc-sections     # Remove unused code
-Wl,--strip-all       # Strip symbols
-Wl,-O3               # Linker optimization
```

## 📏 Expected Sizes

| Stage | Size |
|-------|------|
| Debug build | 40-60 MB |
| Release (-O2) | 15-25 MB |
| **MinSizeRel (-O3 + flags)** | **5-10 MB** |
| **+ UPX compression** | **2-4 MB** |

## 🎯 Build Output

Your executable will be at:
- **Linux/WSL:** `build-windows/rvtt.exe`
- **Windows:** `build-windows\rvtt.exe`

## ✅ Verify Your Build

### Check static linking
```bash
# Should only show system DLLs
x86_64-w64-mingw32-objdump -p build-windows/rvtt.exe | grep "DLL Name"
```

### Check LTO is enabled
```bash
grep CMAKE_INTERPROCEDURAL_OPTIMIZATION build-windows/CMakeCache.txt
# Should show: ON
```

### Check file size
```bash
ls -lh build-windows/rvtt.exe    # Should be 5-10 MB
```

## 🐛 Common Issues

### "LLVM_MINGW_ROOT not set"
```bash
export LLVM_MINGW_ROOT=/path/to/llvm-mingw
```

### "PortAudio not found"
See [PORTAUDIO_WINDOWS.md](PORTAUDIO_WINDOWS.md) for build instructions.

### "Ninja not found"
```bash
sudo apt install ninja-build    # Linux
winget install Ninja-build.Ninja # Windows
```

### Binary too large (>20 MB)
1. Check LTO is enabled
2. Strip manually: `llvm-strip -s build-windows/rvtt.exe`
3. Use UPX: `upx --best --lzma build-windows/rvtt.exe`

## 🎉 Next Steps

1. **Build:** Run `./build-windows.sh` or `build-windows.bat`
2. **Test:** Copy `rvtt.exe` and model file to Windows
3. **Run:** `rvtt.exe models\ggml-base.bin ja`
4. **Distribute:** Package exe + model (no other files needed!)

## 🌟 Features

| Feature | Status |
|---------|--------|
| LLVM-MinGW (clang++) | ✅ |
| CMake + Ninja | ✅ |
| -O3 optimization | ✅ |
| Link-time optimization (LTO) | ✅ |
| Static linking | ✅ |
| Symbol stripping | ✅ |
| Dead code removal | ✅ |
| VS Code integration | ✅ |
| Cross-compilation support | ✅ |
| Native Windows build | ✅ |

## 📞 Support

For detailed information, see the comprehensive guides:
- **Quick Start:** [QUICKSTART_WINDOWS.md](QUICKSTART_WINDOWS.md)
- **Full Guide:** [WINDOWS_BUILD.md](WINDOWS_BUILD.md)
- **Recipe Card:** [WINDOWS_BUILD_RECIPE.txt](WINDOWS_BUILD_RECIPE.txt)

## 🔗 Resources

- **LLVM-MinGW:** https://github.com/mstorsjo/llvm-mingw
- **Ninja:** https://ninja-build.org/
- **UPX:** https://upx.github.io/
- **PortAudio:** http://www.portaudio.com/

---

**Ready to build?** → Start with [QUICKSTART_WINDOWS.md](QUICKSTART_WINDOWS.md)

**Need help?** → See [WINDOWS_BUILD.md](WINDOWS_BUILD.md) for troubleshooting

**Happy building!** 🚀

