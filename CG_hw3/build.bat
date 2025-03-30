@echo off
setlocal enabledelayedexpansion

rem Get root path
set "root_path=%cd%"
cd /d "%root_path%"

rem Create build folders
if not exist build mkdir build
if not exist build\bin mkdir build\bin

rem Set build and toolchain paths
set "build_path=%root_path%\build"
set "toolchain_file=%root_path%\..\vcpkg\scripts\buildsystems\vcpkg.cmake"

rem Check if toolchain file exists
if not exist "!toolchain_file!" (
    echo [ERROR] vcpkg toolchain not found: !toolchain_file!
    pause
    exit /b 1
)

rem Run CMake configure
cmake -B "!build_path!" -S . -DCMAKE_TOOLCHAIN_FILE="!toolchain_file!" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo [ERROR] CMake configuration failed.
    pause
    exit /b %errorlevel%
)

rem Build project
cmake --build "!build_path!" --config Release
if errorlevel 1 (
    echo [ERROR] Build failed.
    pause
    exit /b %errorlevel%
)

echo [SUCCESS] Build completed.
pause
