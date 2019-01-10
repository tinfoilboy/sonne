cmake_minimum_required(VERSION 3.0)

include(CMake/PrecompiledHeader.cmake)
project(computare)

find_package(Threads)

file (GLOB_RECURSE source_files "src/*.cpp")

find_package(PythonInterp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(YAML_CPP_BUILD_TESTS OFF)
set(YAML_CPP_BUILD_TOOLS OFF)
set(FMT_TEST OFF)

add_subdirectory(fmt)
add_subdirectory(yaml-cpp)

add_executable(computare ${source_files})

add_custom_target(
    computare_prebuild
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/prebuild_config.py
    COMMENT "Adding default configuration file to main.cpp"
)

add_dependencies(computare computare_prebuild)

add_custom_command(
    TARGET computare POST_BUILD
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/postbuild_clean.py
    COMMENT "Cleaning up prebuild main step"
)

target_link_libraries(computare fmt::fmt yaml-cpp ${CMAKE_THREAD_LIBS_INIT} stdc++fs)

if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(computare PRIVATE "-Wall")
endif()

if (MSVC)
    target_compile_options(computare PRIVATE "/W4")
endif()

add_precompiled_header(computare pch.hpp FORCEINCLUDE)