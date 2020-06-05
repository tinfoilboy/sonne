cmake_minimum_required(VERSION 3.0)

project(computare VERSION 2.0.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "disable yaml tests")
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "disable yaml tools")
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "disable yaml contrib")
set(FMT_TEST OFF CACHE BOOL "disable fmt tests")
set(FMT_DOC OFF CACHE BOOL "disable fmt docs")

add_subdirectory(fmt)
add_subdirectory(yaml-cpp)

add_executable(computare 
    source/main.cpp
    source/pch.cpp
    source/file.cpp)

target_include_directories(computare PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(computare fmt::fmt yaml-cpp)

if (MSVC)
    target_compile_options(computare PRIVATE "/W4")
else()
    target_compile_options(computare PRIVATE "-Wall")
endif()
