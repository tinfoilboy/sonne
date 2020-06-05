#pragma once

#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <map>
#include <chrono>
#include <memory>
#include <algorithm>
#include <vector>
#include <climits>

#include "cxxopt.hpp"
#include <fmt/format.h>
#include <yaml-cpp/yaml.h>

#ifdef _WIN32
#include <Windows.h>
#define PATH_MAX MAX_PATH
#else
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

/**
 * Print a fatal error to the console and exit the program.
 */
inline static void Fatal(const std::string& message, int code=-1)
{
    fmt::print("[error]: {}\n", message);

    exit(code);
}