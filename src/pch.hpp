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

#include "cxxopt.hpp"
#include <fmt/format.h>
#include <yaml-cpp/yaml.h>

/**
 * Print a fatal error to the console and exit the program.
 */
inline static void Fatal(std::string_view message, int code=-1)
{
    fmt::print("[error]: {}\n", message);

    exit(code);
}