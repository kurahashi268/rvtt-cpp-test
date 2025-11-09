@echo off
echo =====================================
echo Building RVTT with LLVM + Ninja
echo =====================================

setlocal
set PATH="C:\llvm-mingw\bin";%PATH%
set CC=clang
set CXX=clang++

rmdir /s /q build
mkdir build
cd build

cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
@REM cmake --build . --config Release
cmake --build . --config Release -- -j1
endlocal

copy ..\dll\* .