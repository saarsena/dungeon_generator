cmake_minimum_required(VERSION 3.9)
project(tiling_wfc_example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(simple_example simple_example.cpp)
target_link_libraries(simple_example PRIVATE tiling_wfc_static)
