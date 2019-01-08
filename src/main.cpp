#include "pch.hpp"

#include "counter.hpp"

int main(int argc, char** argv)
{
    Info("computare 1.0.0");
    Info("a simple and fast way to count lines of code.");
    Info("");

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

        Info("counting file " + file + "...");

        auto start = std::chrono::system_clock::now();

        Counter counter(file);
        
        FileInfo info = counter.Count();

        auto end = std::chrono::system_clock::now();

        std::chrono::duration<double> elapsed = end - start;

        Info("finished counting in " + std::to_string(elapsed.count()) + "s!");

        PrintSingleFileInfo(info);

        return 0;
    }

    return 0;
}