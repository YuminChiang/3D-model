cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

project(CG_hw3)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)
find_package(glm REQUIRED)

file(GLOB SRC_FILES
    "${PROJECT_SOURCE_DIR}/src/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${CMAKE_PROJECT_NAME} 
    PRIVATE GLEW::GLEW GLUT::GLUT OpenGL::GL glm::glm ${OpenCV_LIBS})

target_include_directories(${CMAKE_PROJECT_NAME} 
    PRIVATE ${OpenCV_INCLUDE_DIRS}
)

add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/TestModels_HW3"
            "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/TestModels_HW3"
)

add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/TestTextures_HW3"
            "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/TestTextures_HW3"
)

add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/shaders"
            "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/shaders"
)

add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/textures"
            "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/textures"
)
