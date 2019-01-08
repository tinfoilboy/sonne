cmake_minimum_required(VERSION 3.0)

include(CMake/PrecompiledHeader.cmake)
project(computare)

find_package(Threads)

file (GLOB_RECURSE source_files "src/*.cpp")

set (CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(fmt)
add_subdirectory(yaml-cpp)

add_executable(computare ${source_files})

target_link_libraries(computare fmt::fmt yaml-cpp ${CMAKE_THREAD_LIBS_INIT} stdc++fs)

add_precompiled_header(computare pch.hpp FORCEINCLUDE)