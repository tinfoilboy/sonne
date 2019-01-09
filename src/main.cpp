#include "pch.hpp"

#include "counter.hpp"
#include "config.hpp"
#include "directory_counter.hpp"

/*
 * Main command-line entry point for the program.
 */
int main(int argc, char** argv)
{
    cxxopts::Options options(
        "computare",
        "a simple and configurable program for counting lines in files"
    );

    options.add_options()
        (
            "f,file",
            "The file to count lines from",
            cxxopts::value<std::string>()
        )
        (
            "d,dir",
            "The directory to recursively walk and count",
            cxxopts::value<std::string>()
        )
        (
            "c,config",
            "A path to the config file to use for computare",
            cxxopts::value<std::string>()
        )
        (
            "b,block-size",
            "Sets the amount of characters to read per block, may increase speed",
            cxxopts::value<size_t>()
        )
        (
            "s,skip-hidden",
            "Determines whether hidden files/directories should be skipped over"
        )
        (
            "h,help",
            "Print help for the program"
        );

    auto result = options.parse(argc, argv);

    if (result.count("h"))
    {
        fmt::print("{}\n", options.help());

        return 0;
    }

    fmt::print("computare 1.0.0\n");
    fmt::print("a simple and configurable program for counting lines in files.\n");
    fmt::print("use -h or --help to see how to use.\n");
    fmt::print("\n");

    char exePath[PATH_MAX + 1];

#ifdef __linux__
    realpath(argv[0], exePath);
#elif _WIN32
    _fullpath(exePath, argv[0], sizeof(exePath));
#endif

    std::string rootPath = std::experimental::filesystem::path(exePath).parent_path().string();

    std::string configPath = ".computare.yml";

    Config config;

    std::string rootConfig = fmt::format("{}/.computare.yml", rootPath);

    if (std::experimental::filesystem::exists(configPath))
        config.Parse(configPath);

    if (std::experimental::filesystem::exists(rootConfig))
        config.Parse(rootConfig);

    // parse a custom config from the passed in path if exists
    if (result.count("c"))
    {
        configPath = result["c"].as<std::string>();
    
        if (std::experimental::filesystem::exists(configPath))
            config.Parse(configPath);
    }

    // set in the configuration whether to ignore hidden files
    if (result.count("s"))
        config.SetIgnoreHidden(result["s"].as<bool>());

    // set the processing block size in the config
    if (result.count("b"))
        config.SetBlockSize(result["b"].as<size_t>());

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

        fmt::print("\n");

        PrintSingleFileInfo(info);

        return 0;
    }

    // run the recursive directory counter on the passed in directory
    if (result.count("d"))
    {
        std::string dir = result["d"].as<std::string>();

        std::string potentialConfig = fmt::format("{}/.computare.yml", dir);

        if (std::experimental::filesystem::exists(potentialConfig))
        {
            fmt::print("loading configuration file from {}...\n", potentialConfig);
            
            config.Parse(potentialConfig);
        }

        fmt::print("counting directory {}...\n", dir);

        auto start = std::chrono::system_clock::now();

        DirectoryCounter counter(dir);

        DirectoryInfo info = counter.Run(config);

        auto end = std::chrono::system_clock::now();

        std::chrono::duration<double> elapsed = end - start;

        fmt::print(
            "finished counting in {}ms!\n",
            std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()
        );

        fmt::print("\n");

        PrintDirectoryInfo(info);

        return 0;
    }

    return 0;
}