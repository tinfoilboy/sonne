cmake_minimum_required(VERSION 3.0)

project (computare LANGUAGES CXX VERSION 2.0.0)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (COMPUTARE_BASE_SOURCE 
    ${CMAKE_SOURCE_DIR}/source/file.cpp
    ${CMAKE_SOURCE_DIR}/source/directory_counter.cpp
    ${CMAKE_SOURCE_DIR}/source/counter.cpp
    ${CMAKE_SOURCE_DIR}/source/config.cpp)

set (YAML_CPP_BUILD_TESTS OFF CACHE BOOL "disable yaml tests")
set (YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "disable yaml tools")
set (YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "disable yaml contrib")
set (FMT_TEST OFF CACHE BOOL "disable fmt tests")
set (FMT_DOC OFF CACHE BOOL "disable fmt docs")
set (JSON_BuildTests OFF CACHE INTERNAL "")

option (COMPUTARE_BUILD_TESTS "Whether to enable the test suite for computare" ON)

add_subdirectory (extern/fmt)
add_subdirectory (extern/Catch2)
add_subdirectory (extern/json)

add_executable (computare 
    ${COMPUTARE_BASE_SOURCE}
    ${CMAKE_SOURCE_DIR}/source/main.cpp)

target_include_directories (computare PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries (computare nlohmann_json::nlohmann_json fmt::fmt)

if (MSVC)
    target_compile_options (computare PRIVATE "/W4")
else()
    target_compile_options (computare PRIVATE "-Wall")
endif()

if (COMPUTARE_BUILD_TESTS)
    add_executable (computare_test
        ${COMPUTARE_BASE_SOURCE}
        ${CMAKE_SOURCE_DIR}/tests/main.cpp)

    target_include_directories (computare_test PUBLIC ${CMAKE_SOURCE_DIR}/include)

    target_link_libraries (computare_test fmt::fmt nlohmann_json::nlohmann_json Catch2::Catch2)

    file (COPY ${CMAKE_SOURCE_DIR}/tests/samples DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file (COPY ${CMAKE_SOURCE_DIR}/tests/dir_walk DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()
