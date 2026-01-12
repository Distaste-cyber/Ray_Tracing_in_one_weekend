
cmake_minimum_required(VERSION 3.16)
project(tiny_renderer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include_directories(include)
add_executable(tiny_renderer
    src/main.cpp
    include/tgaimage.cpp
    include/our_gl.cpp
)
