#pragma once

#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <string_view>
#include <chrono>

#include "cxxopt.hpp"
#include "termcolor.hpp"

/**
 * Print a simple Info log line to the console.
 * 
 * Leave empty for a single newline.
 */
inline static void Info(std::string_view message)
{
    std::cout << termcolor::yellow
              << message
              << termcolor::reset
              << std::endl;
}

/**
 * Print a fatal error to the console and exit the program.
 */
inline static void Fatal(std::string_view message, int code=-1)
{
    std::cout << termcolor::red
              << message
              << termcolor::reset
              << std::endl;

    exit(code);
}