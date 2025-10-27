@echo off
REM Native Windows build script using LLVM-MinGW
REM Run this on Windows with LLVM-MinGW and Ninja installed

setlocal enabledelayedexpansion

echo === RVTT-CPP Windows Build Script ===
echo.

REM Check if clang++ is available
where clang++ >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Error: clang++ not found in PATH!
    echo.
    echo Please install LLVM-MinGW from:
    echo   https://github.com/mstorsjo/llvm-mingw/releases
    echo.
    echo And add it to your PATH:
    echo   set PATH=C:\path\to\llvm-mingw\bin;%%PATH%%
    echo.
    pause
    exit /b 1
)

REM Check if Ninja is available
where ninja >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Error: Ninja build system not found!
    echo.
    echo Install Ninja from:
    echo   https://github.com/ninja-build/ninja/releases
    echo.
    echo Or use: winget install Ninja-build.Ninja
    echo.
    pause
    exit /b 1
)

echo [OK] LLVM-MinGW clang++ found
echo [OK] Ninja build system found
echo.

REM Build directory
set BUILD_DIR=build-windows

REM Clean build if requested
if "%1"=="clean" (
    echo Cleaning build directory...
    if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
)

REM Create build directory
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM Configure
echo === Configuring CMake ===
cmake --preset windows-x64-native
if %ERRORLEVEL% NEQ 0 (
    echo Configuration failed!
    pause
    exit /b 1
)

REM Build
echo.
echo === Building ===
cmake --build --preset windows-x64-native
if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    pause
    exit /b 1
)

REM Check if build was successful
if exist "%BUILD_DIR%\rvtt.exe" (
    echo.
    echo === Build Successful! ===
    echo.
    
    REM Show file size
    for %%A in ("%BUILD_DIR%\rvtt.exe") do set SIZE=%%~zA
    set /a SIZE_MB=!SIZE! / 1024 / 1024
    echo Executable: %BUILD_DIR%\rvtt.exe
    echo Size: !SIZE_MB! MB (approx)
    
    REM Strip executable
    echo.
    echo Applying additional optimizations...
    llvm-strip -s "%BUILD_DIR%\rvtt.exe" 2>nul
    
    REM Show final size
    for %%A in ("%BUILD_DIR%\rvtt.exe") do set FINAL_SIZE=%%~zA
    set /a FINAL_SIZE_MB=!FINAL_SIZE! / 1024 / 1024
    echo Final size: !FINAL_SIZE_MB! MB (approx)
    
    REM Optional UPX compression
    where upx >nul 2>nul
    if %ERRORLEVEL% EQU 0 (
        echo.
        set /p COMPRESS="UPX found! Compress executable further? (y/N): "
        if /i "!COMPRESS!"=="y" (
            upx --best --lzma "%BUILD_DIR%\rvtt.exe"
            for %%A in ("%BUILD_DIR%\rvtt.exe") do set UPX_SIZE=%%~zA
            set /a UPX_SIZE_MB=!UPX_SIZE! / 1024 / 1024
            echo Compressed size: !UPX_SIZE_MB! MB (approx)
        )
    ) else (
        echo.
        echo Tip: Install UPX for even smaller binaries from:
        echo   https://upx.github.io/
    )
    
    echo.
    echo === Done! ===
    echo.
    echo Your Windows executable is ready at:
    echo   %BUILD_DIR%\rvtt.exe
    echo.
    echo To run, you'll need:
    echo   - The Whisper model file (e.g., models\ggml-base.bin)
    echo.
    
) else (
    echo Build failed! Check the output above for errors.
    pause
    exit /b 1
)

pause

