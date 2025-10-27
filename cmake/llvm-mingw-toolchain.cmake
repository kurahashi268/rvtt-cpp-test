# LLVM-MinGW Toolchain File for Windows Cross-Compilation
# 
# Usage:
#   Set LLVM_MINGW_ROOT environment variable to your LLVM-MinGW installation path
#   Example: export LLVM_MINGW_ROOT=/path/to/llvm-mingw
#
# Or download from: https://github.com/mstorsjo/llvm-mingw/releases

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# LLVM-MinGW installation path
if(DEFINED ENV{LLVM_MINGW_ROOT})
    set(LLVM_MINGW_ROOT $ENV{LLVM_MINGW_ROOT})
else()
    message(FATAL_ERROR 
        "LLVM_MINGW_ROOT environment variable not set!\n"
        "Please set it to your LLVM-MinGW installation directory.\n"
        "Example: export LLVM_MINGW_ROOT=/path/to/llvm-mingw"
    )
endif()

# Architecture (can be overridden)
if(NOT DEFINED LLVM_MINGW_ARCH)
    set(LLVM_MINGW_ARCH "x86_64")
endif()

set(TOOLCHAIN_PREFIX ${LLVM_MINGW_ROOT}/bin/${LLVM_MINGW_ARCH}-w64-mingw32)

# Compiler settings
set(CMAKE_C_COMPILER ${LLVM_MINGW_ROOT}/bin/${LLVM_MINGW_ARCH}-w64-mingw32-clang)
set(CMAKE_CXX_COMPILER ${LLVM_MINGW_ROOT}/bin/${LLVM_MINGW_ARCH}-w64-mingw32-clang++)
set(CMAKE_RC_COMPILER ${LLVM_MINGW_ROOT}/bin/${LLVM_MINGW_ARCH}-w64-mingw32-windres)
set(CMAKE_AR ${LLVM_MINGW_ROOT}/bin/llvm-ar)
set(CMAKE_RANLIB ${LLVM_MINGW_ROOT}/bin/llvm-ranlib)
set(CMAKE_STRIP ${LLVM_MINGW_ROOT}/bin/llvm-strip)

# Target environment
set(CMAKE_FIND_ROOT_PATH ${LLVM_MINGW_ROOT}/${LLVM_MINGW_ARCH}-w64-mingw32)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Optimization flags for smallest binary
set(OPTIMIZATION_FLAGS "-O3 -march=x86-64 -mtune=generic -flto=thin -ffunction-sections -fdata-sections")
set(LINK_FLAGS "-static -static-libgcc -static-libstdc++ -s -Wl,--gc-sections -Wl,--strip-all -Wl,-O3 -flto=thin")

# Apply flags
set(CMAKE_C_FLAGS_INIT "${OPTIMIZATION_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${OPTIMIZATION_FLAGS}")
set(CMAKE_C_FLAGS_MINSIZEREL_INIT "-DNDEBUG")
set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT "-DNDEBUG")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${LINK_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${LINK_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "${LINK_FLAGS}")

# Windows-specific settings
set(CMAKE_EXECUTABLE_SUFFIX ".exe")
set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".dll")

# Force static linking
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

