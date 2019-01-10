#pragma once

#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <string_view>
#include <map>
#include <chrono>
#include <memory>
#include <algorithm>
#include <vector>
#include <queue>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <filesystem>
#include <climits>

#include "cxxopt.hpp"
#include <fmt/format.h>
#include <yaml-cpp/yaml.h>

namespace fs = std::filesystem;

#ifdef _WIN32
#include <Windows.h>
#define PATH_MAX MAX_PATH
#endif

/**
 * Print a fatal error to the console and exit the program.
 */
inline static void Fatal(std::string_view message, int code=-1)
{
    fmt::print("[error]: {}\n", message);

    exit(code);
}