#include "pch.hpp"

#include "counter.hpp"
#include "config.hpp"

/*
 * Main command-line entry point for the program.
 */
int main(int argc, char** argv)
{
    fmt::print("computare 1.0.0\n");
    fmt::print("a simple and fast way to count lines of code.\n");
    fmt::print("\n");

    cxxopts::Options options(
        "computare",
        "A fast and simple line of code counter."
    );

    options.add_options()
        (
            "g,no-ignore",
            "Stops computare from using your gitignore"
        )
        (
            "f,file",
            "The file to count lines from",
            cxxopts::value<std::string>()
        )
        (
            "d,dir",
            "The directory to recursively walk and count",
            cxxopts::value<std::string>()
        );

    auto result = options.parse(argc, argv);

    Config config;

    config.Parse(".computare.yml");

    // get the ignore flag to see whether to parse gitignore
    bool useIgnore = !(result["no-ignore"].as<bool>());

    // start by parsing the gitignore file for use in the counter
    if (useIgnore)
    {
        // todo: implement this! doing counting first
    }
    
    // run counter on a single file and exit when done
    if (result.count("f"))
    {
        std::string file = result["f"].as<std::string>();

        fmt::print("counting file {}...\n", file);

        auto start = std::chrono::system_clock::now();

        Counter counter(file);
        
        FileInfo info = counter.Count(config);

        auto end = std::chrono::system_clock::now();

        std::chrono::duration<double> elapsed = end - start;

        fmt::print(
            "finished counting in {}ms!\n",
            std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()
        );

        PrintSingleFileInfo(info);

        return 0;
    }

    return 0;
}