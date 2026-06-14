cmake_minimum_required(VERSION 3.28)
 
project(HairlossEngine
    DESCRIPTION "Game Engine using C++ and SDL3"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

add_subdirectory(vendor)
add_subdirectory(src)