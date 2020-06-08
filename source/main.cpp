#include "sonne/pch.hpp"

#include <cxxopts.hpp>

#include "sonne/file.hpp"
#include "sonne/config_generator.hpp"
#include "sonne/config.hpp"
#include "sonne/counter.hpp"
#include "sonne/directory_counter.hpp"

int main(int argc, char** argv)
{
    cxxopts::Options options(
        "Sonne",
        "A fast and configurable program for counting lines of code."
    );
    
    std::vector<std::string> positional; // positional arguments for the counter

    options.add_options()
        ("c,config", "A path to a config file to load before counting", cxxopts::value<std::string>())
        ("s,skip-hidden", "Determines whether hidden files/directories should be skipped over")
        ("h,help", "Print help for the program")
        ("positional", "Positional parameters for counting paths", cxxopts::value<std::vector<std::string>>(positional));

    auto result = options.parse(argc, argv);

    if (result.count("h"))
    {
        fmt::print("{}\n", options.help());

        return 0;
    }

    fmt::print("Sonne 2.0.0\n");
    fmt::print("A fast and configurable program for counting lines of code.\n");
    fmt::print("use -h or --help to see how to use.\n\n");

    std::shared_ptr<Sonne::Config> config = nullptr;

    std::string globalConfigPath = "";

    // default path for the global config should be the users home directory, or User folder in Windows.
#ifdef __linux__
    globalConfigPath = fmt::format("{}/.computare.yml", getenv("HOME"));
#elif _WIN32
    globalConfigPath = fmt::format("{}/.computare.yml", getenv("USERPROFILE"));
#endif

    Sonne::Entry configFile = Sonne::GetFSEntry(globalConfigPath);

    // the config file does not exist, thus we should create the default and write the file to the global path
    if (!configFile.isValid)
    {
        config = Sonne::GenerateDefaultConfig();

        config->Write(globalConfigPath);
    }
    else
    {
        config->Parse(globalConfigPath);
    }

    // parse a custom config from the passed in path if exists
    if (result.count("c"))
    {
        const std::string& configPath = result["c"].as<std::string>();

        Sonne::Entry newConfig = Sonne::GetFSEntry(globalConfigPath);

        if (newConfig.isValid)
        {
            config->Parse(configPath);
        }
    }

    // set in the configuration whether to ignore hidden files
    if (result.count("s"))
    {
        config->SetIgnoreHidden(result["s"].as<bool>());
    }
}