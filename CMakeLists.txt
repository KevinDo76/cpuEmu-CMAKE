cmake_minimum_required(VERSION 3.5.0)

project(first VERSION 0.1.0 LANGUAGES C CXX)

set(SOURCE_FILE src/main.cpp src/cpu.cpp)

add_executable(first ${SOURCE_FILE})

