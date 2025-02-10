cmake_minimum_required(VERSION 3.5.0)
project(raytracer VERSION 0.1.0 LANGUAGES C CXX)

# Specify the compiler
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

add_executable(raytracer main.cpp)

