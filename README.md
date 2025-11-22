# 3D-model
https://www.youtube.com/watch?v=pmI8FD3t-Bc

A collection of projects for computer graphics assignments leveraging modern OpenGL and C++. Each subfolder (CG_hw1, CG_hw2, CG_hw3) contains implementations related to 3D modeling, camera manipulation, transformation, and rendering. These assignments use libraries such as GLEW, GLUT, and GLM.

## Features

- 3D model loading and rendering (by assignment)
- Interactive camera manipulation
- Custom shaders and transformations
- Support for test models per assignment

## Folder Structure

- **CG_hw1/** – Homework 1: entry point in `src/CG2023_HW1.cpp`.
- **CG_hw2/** – Homework 2: camera logic in `src/camera.h`/`.cpp`.
- **CG_hw3/** – Homework 3: camera logic in `src/camera.h`/`.cpp`.

Each has its own `CMakeLists.txt` using required libraries.

## Setup

1. **Clone vcpkg and install dependencies:**
   ```sh
   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat   
   ./vcpkg install glew freeglut glm opencv4 pkgconf
   ```

2. **Build an Assignment:**

   Build (replace `CG_hw1` with `CG_hw2` or `CG_hw3` as appropriate):

   ```sh
   cd CG_hw1
   mkdir build && cd build
   cmake .. -DCMAKE_TOOLCHAIN_FILE=[path_to_vcpkg]/scripts/buildsystems/vcpkg.cmake
   cmake --build .
   ```

## Requirements

- Modern C++ (C++20)
- OpenGL
- GLEW
- GLUT
- GLM
- vcpkg for package management

## Usage

Run the compiled executable in the appropriate build directory:
```sh
./CG_hw1
```

## License

This project is intended for educational use by the owner.
