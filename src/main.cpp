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

    fmt::print("computare 1.0.1\n");
    fmt::print("a simple and configurable program for counting lines in files.\n");
    fmt::print("use -h or --help to see how to use.\n\n");

    std::string globalConfig = "";
    std::string configPath   = ".computare.yml";

    Config config;

// on linux systems, the config should be placed in the user home directory
// which evaluates to ~ in paths, thus place a default config there if one
// doesn't exist, as well as load the config at that path
#ifdef __linux__
    globalConfig = fmt::format("{}/.computare.yml", getenv("HOME"));
#elif _WIN32
    globalConfig = fmt::format("{}/.computare.yml", getenv("USERPROFILE"));
#endif

    fmt::print("global computare config location: {}\n", globalConfig);

    if (!fs::exists(globalConfig))
    {
        std::ofstream out(globalConfig);

        std::string config = "# Whether or not to ignore hidden files from the counter\nignore-hidden: true\n# The amount of characters/bytes to read from a file per iteration until the\n# end of the file. Increasing this value may speed up the counting process at\n# the price of memory usage for storing the blocks.\nblock-size: 131072\n# A block of language definitions based on extension that defines the name of\n# the language, as well as its comment styles, any of these can be omitted\n# aside from the extensions and language fields.\nlanguages:\n  -\n    extensions: [ 'c', 'cpp', 'cxx', 'cc', 'C' ]\n    language: C/C++ Source\n    line-comment: //\n    block-comment-begin: /*\n    block-comment-end: '*/'\n  -\n    extensions: [ 'h', 'hpp', 'hxx', 'hh', 'H' ]\n    language: C/C++ Header\n    line-comment: //\n    block-comment-begin: /*\n    block-comment-end: '*/'\n  -\n    extensions: [ 'py' ]\n    language: Python\n    line-comment: '#'\n  -\n    extensions: [ 'java' ]\n    language: Java\n    line-comment: //\n    block-comment-begin: /*\n    block-comment-end: '*/'\n  -\n    extensions: [ 'kt', 'kts' ]\n    language: Kotlin\n    line-comment: //\n    block-comment-begin: /*\n    block-comment-end: '*/'\n  -\n    extensions: [ 'rs' ]\n    language: Rust\n    line-comment: //\n    block-comment-begin: /*\n    block-comment-end: '*/'\n  -\n    extensions: [ 'js' ]\n    language: JavaScript\n    line-comment: //\n    block-comment-begin: /*\n    block-comment-end: '*/'\n  -\n    extensions: [ 'css' ]\n    language: CSS\n    block-comment-begin: /*\n    block-comment-end: '*/'\n  -\n    extensions: [ 'html', 'htm', 'xhtml' ]\n    language: HTML\n    block-comment-begin: <!--\n    block-comment-end: -->\n  -\n    extensions: [ 'ts', 'tsx' ]\n    language: TypeScript\n    line-comment: //\n    block-comment-begin: /*\n    block-comment-end: '*/'\n  -\n    extensions: [ 'rb' ]\n    language: Ruby\n    line-comment: '#'\n  -\n    extensions: [ 'lua' ]\n    language: Lua\n    line-comment: --\n    block-comment-begin: --[[\n    block-comment-end: --]]\n  -\n    extensions: [ 'd' ]\n    language: D\n    line-comment: //\n    block-comment-begin: /*\n    block-comment-end: '*/'\n  -\n    extensions: [ 'cs' ]\n    language: C#\n    line-comment: //\n    block-comment-begin: /*\n    block-comment-end: '*/'\n  -\n    extensions: [ 'php', 'php3', 'php4', 'php5' ]\n    language: PHP\n    line-comment: //\n    block-comment-begin: /*\n    block-comment-end: '*/'\n  -\n    extensions: [ 'json' ]\n    language: JSON\n  -\n    extensions: [ 'go' ]\n    language: Go\n    line-comment: //\n    block-comment-begin: /*\n    block-comment-end: '*/'\n  -\n    extensions: [ 'swift' ]\n    language: Swift\n    line-comment: //\n    block-comment-begin: /*\n    block-comment-end: '*/'\n  -\n    extensions: [ 'h', 'm', 'mm', 'M' ]\n    language: Objective C\n    line-comment: //\n    block-comment-begin: /*\n    block-comment-end: '*/'\n  -\n    extensions: [ 'yml', 'yaml' ]\n    language: YAML\n    line-comment: '#'\n  -\n    extensions: [ 'ini' ]\n    language: INI Configuration\n    line-comment: ;";

        out << config;

        out.close();
    }

    config.Parse(globalConfig);

    if (fs::exists(configPath))
        config.Parse(configPath);

    // parse a custom config from the passed in path if exists
    if (result.count("c"))
    {
        configPath = result["c"].as<std::string>();
    
        if (fs::exists(configPath))
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

        fmt::print("counting in file '{}'\n", file);

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

        if (fs::exists(potentialConfig))
        {
            fmt::print("loading configuration file from '{}'\n\n", potentialConfig);
            
            config.Parse(potentialConfig);
        }
        else
            fmt::print("\n");

        fmt::print("counting lines from files in directory '{}'\n", dir);

        auto start = std::chrono::system_clock::now();

        DirectoryCounter counter(dir);

        DirectoryInfo info = counter.Run(config);

        auto end = std::chrono::system_clock::now();

        std::chrono::duration<double> elapsed = end - start;

        fmt::print(
            "finished counting in {}ms\n",
            std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()
        );

        fmt::print("\n");

        PrintDirectoryInfo(info);

        return 0;
    }

    return 0;
}