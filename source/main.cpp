#include "sonne/pch.hpp"

#include <cxxopts.hpp>

#include "sonne/file.hpp"
#include "sonne/config_generator.hpp"
#include "sonne/config.hpp"
#include "sonne/counter.hpp"
#include "sonne/directory_counter.hpp"

size_t get_console_columns()
{
    size_t columns = 0;

#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO info = {};

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);

    columns = (static_cast<size_t>(info.srWindow.Right) - static_cast<size_t>(info.srWindow.Left)) + 1;
#else
    struct winsize size;

    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);

    columns = static_cast<size_t>(size.ws_col);
#endif

    return columns;
}

void print_info_header(size_t columns)
{
    // the size of each cell in the header minus four for the seperators and spaces
    size_t cellWidth = static_cast<size_t>(std::roundf(static_cast<float>(columns) / 6.0f)) - 4;

    fmt::print("{:->{}}\n", "", ((cellWidth + 4) * 6) - 5);

    fmt::print("| {: <{}} |", "Language", cellWidth);
    fmt::print(" {: >{}} |", "Files", cellWidth);
    fmt::print(" {: >{}} |", "Empty", cellWidth);
    fmt::print(" {: >{}} |", "Code", cellWidth);
    fmt::print(" {: >{}} |", "Comment", cellWidth);
    fmt::print(" {: >{}} |\n", "Total", cellWidth);

    fmt::print("{:->{}}\n", "", ((cellWidth + 4) * 6) - 5);
}

void print_language_entry(Sonne::CountInfo& info, size_t columns)
{
    // the size of each cell in the header minus four for the seperators and spaces
    size_t cellWidth = static_cast<size_t>(std::roundf(static_cast<float>(columns) / 6.0f)) - 4;

    if (info.language.size() > cellWidth)
    {
        info.language = info.language.substr(0, cellWidth); // truncate the language string if it is too long
    }

    fmt::print("| {: <{}} |", info.language, cellWidth);
    fmt::print(" {: >{}} |", info.files, cellWidth);
    fmt::print(" {: >{}} |", info.emptyLines, cellWidth);
    fmt::print(" {: >{}} |", info.codeLines, cellWidth);
    fmt::print(" {: >{}} |", info.commentLines, cellWidth);
    fmt::print(" {: >{}} |\n", info.totalLines, cellWidth);
}

void print_table_end(size_t columns)
{
    // the size of each cell in the header minus four for the seperators and spaces
    size_t cellWidth = static_cast<size_t>(std::roundf(static_cast<float>(columns) / 6.0f)) - 4;

    fmt::print("{:->{}}\n", "", ((cellWidth + 4) * 6) - 5);
}

int main(int argc, char** argv)
{
    fmt::print("\n"); // print a new line to separate from the command input

    cxxopts::Options options(
        "Sonne",
        "A fast and configurable program for counting lines of code."
    );
    
    std::vector<std::string> positional; // positional arguments for the counter

    options.add_options()
        ("h,help", "Print help for the program")
        ("i,ignore-hidden", "Determines whether hidden files/directories should be skipped over")
        ("c,columns", "Amount of columns to base print off of", cxxopts::value<size_t>())
        ("input", "Input path for the program", cxxopts::value<std::string>())
        ("positional", "Positional parameters for counting paths", cxxopts::value<std::vector<std::string>>(positional));

    options.parse_positional({ "input" });
    auto result = options.parse(argc, argv);

    if (result.count("h"))
    {
        fmt::print("{}\n", options.help());

        return 0;
    }

    std::shared_ptr<Sonne::Config> config = nullptr;

    std::string globalConfigPath = "";

    // default path for the global config should be the users home directory, or User folder in Windows.
#ifdef __linux__
    globalConfigPath = fmt::format("{}/.sonne.json", getenv("HOME"));
#elif _WIN32
    globalConfigPath = fmt::format("{}/.sonne.json", getenv("USERPROFILE"));
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
        config = std::make_shared<Sonne::Config>();

        config->Parse(globalConfigPath);
    }

    // parse column count from command line if specified
    if (result.count("c"))
    {
        config->SetColumns(result["columns"].as<size_t>());
    }

    size_t columns = config->GetColumns();

    fmt::print("{: ^{}}\n", "Sonne 2.1.1", columns);
    fmt::print("{: ^{}}\n\n", "Simple extensible LOC counter.", columns);

    // set in the configuration whether to ignore hidden files
    if (result.count("s"))
    {
        config->SetIgnoreHidden(result["s"].as<bool>());
    }

    if (result.count("input"))
    {
        std::string input = result["input"].as<std::string>();

        Sonne::Entry entry = Sonne::GetFSEntry(input);

        if (entry.isValid)
        {
            fmt::print("{: ^{}}\n\n", entry.fullPath, columns);

            print_info_header(columns);

            if (entry.isDirectory)
            {
                Sonne::DirectoryCounter counter(entry.fullPath, config);

                Sonne::DirectoryInfo info = counter.Run();

                for (auto& language : info.totals)
                {
                    if (language.second.language == "Totals")
                    {
                        continue; // print totals at the end of the info separate from other langs
                    }

                    print_language_entry(language.second, columns);
                }

                print_table_end(columns);

                print_language_entry(info.totals.at("Totals"), columns);
            }
            else
            {
                Sonne::Counter counter(entry.fullPath);

                Sonne::CountInfo info = counter.Count(config);

                print_language_entry(info, columns);

                Sonne::CountInfo totals = info; // create the total counts to print as well

                totals.language = "Total";

                print_language_entry(totals, columns);
            }

            print_table_end(columns);
        }
        else
        {
            fmt::print("Invalid file or directory given at: {}\n", input);
        }
    }
    else
    {
        fmt::print("{: ^{}}", "Provide a file or directory to count!", columns);
    }

    fmt::print("\n"); // finish with a new line

    return 0;
}
