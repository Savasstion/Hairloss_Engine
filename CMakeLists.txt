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

# Treat source files as UTF-8 and encode string literals as UTF-8 on MSVC
# (GCC/Clang already default to this). Set after vendor so third-party code keeps its own flags.
add_compile_options($<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/utf-8>)

add_subdirectory(tests)
add_subdirectory(src)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	PROPERTY VS_STARTUP_PROJECT HairlossEngine
)