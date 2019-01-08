cmake_minimum_required(VERSION 3.0)
project(computare)

file (GLOB_RECURSE source_files "src/*.cpp")

set (CMAKE_CXX_STANDARD 17)

add_executable(computare ${source_files})