cmake_minimum_required(VERSION 3.0)

include(CMake/PrecompiledHeader.cmake)
project(computare)

file (GLOB_RECURSE source_files "src/*.cpp")

set (CMAKE_CXX_STANDARD 17)

add_subdirectory(fmt)
add_subdirectory(yaml-cpp)

add_executable(computare ${source_files})

target_link_libraries(computare fmt::fmt yaml-cpp)

add_precompiled_header(computare pch.hpp FORCEINCLUDE)