cmake_minimum_required(VERSION 3.28)
 
project(HairlossEngine
    DESCRIPTION "Game Engine using C++ and SDL3"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

add_subdirectory(vendor)
add_subdirectory(tests)
add_subdirectory(src)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTY VS_STARTUP_PROJECT HairlossEngine
)

target_sources(${PROJECT_NAME} PRIVATE
    CMakeLists.txt
)