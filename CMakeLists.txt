cmake_minimum_required(VERSION 3.21)
project(loliEngine)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_COMPILE_FEATURES fconcepts)

find_package(SDL2 REQUIRED)
add_executable(loliEngine main.cpp src/LoliEngine.hpp)
target_link_libraries(loliEngine SDL2)
target_link_libraries(loliEngine GL)
