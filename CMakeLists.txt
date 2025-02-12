cmake_minimum_required(VERSION 3.10)
project(raytracer VERSION 0.1.0 LANGUAGES C CXX)

# Specify the compiler
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

# Set the CMake toolchain file for vcpkg
# set(CMAKE_TOOLCHAIN_FILE "/Users/tadzio/git/vcpkg/scripts/buildsystems/vcpkg.cmake")

# Find packages
find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
# find_package(glad CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# Add the executable
add_executable(raytracer main.cpp)

# Link libraries
target_link_libraries(raytracer PRIVATE 
    imgui::imgui
    glfw
    OpenGL::GL)

