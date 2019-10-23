cmake_minimum_required(VERSION 3.0)

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

include(cotire)
project(computare)

find_package(Threads)

file (GLOB_RECURSE source_files "src/*.cpp")

find_package(PythonInterp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "disable yaml tests")
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "disable yaml tools")
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "disable yaml contrib")
set(FMT_TEST OFF CACHE BOOL "disable fmt tests")
set(FMT_DOC OFF CACHE BOOL "disable fmt docs")

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
set_target_properties(computare PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "src/pch.hpp")

if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(computare PRIVATE "-Wall")
endif()

if (MSVC)
    target_compile_options(computare PRIVATE "/W4")
endif()

cotire(computare)
