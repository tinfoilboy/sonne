#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <computare/pch.hpp>

#include <computare/file.hpp>
#include <computare/config.hpp>
#include <computare/directory_counter.hpp>

using namespace Computare;

inline void CountSubdirectory(Entry& directory, size_t nestAmt, size_t& totalEntries)
{
    for (size_t index = 0; index < directory.children.size(); index++)
    {
        Entry entry = directory.children[index];

        std::string type = (entry.isDirectory) ? "DIRECTORY" : "FILE";

        fmt::print("{}- {}; {}\n", std::string(nestAmt, ' '), type, entry.fullPath);
        
        if (entry.isDirectory && !entry.children.empty())
        {
            CountSubdirectory(entry, nestAmt + 1, totalEntries);

            totalEntries += entry.children.size();
        }
    }
}

TEST_CASE("filesystem custom functions work correctly")
{
    SECTION("individual entry grabbing works correctly")
    {
        Entry entry = GetFSEntry("samples/test.py");

        REQUIRE(entry.isValid == true);

        // file size of the test.py file (unless updated) should always be 142
        REQUIRE(entry.fileSize == 142);
    }

    SECTION("directory crawling works")
    {
        std::vector<Entry> entries = WalkDirectory("dir_walk");
        
        size_t totalEntries = entries.size();

        for (size_t index = 0; index < entries.size(); index++)
        {
            Entry entry = entries[index];

            std::string type = (entry.isDirectory) ? "DIRECTORY" : "FILE";

            fmt::print("- {}; {}\n", type, entry.fullPath);
            
            if (entry.isDirectory && !entry.children.empty())
            {
                CountSubdirectory(entry, 1, totalEntries);

                totalEntries += entry.children.size();
            }
        }

        // the initial amount of entries including directories should be 8 entries
        REQUIRE(totalEntries == 8);
    }
}

TEST_CASE("directory counter works properly")
{
    // separate the previous case with two newlines
    fmt::print("\n\n");

    std::shared_ptr<Config> config = std::make_shared<Config>();

    SECTION("path walking returns correct entries")
    {
        Entry dir = GetFSEntry("samples");

        DirectoryCounter counter(dir.fullPath, config);

        size_t ignoredFiles = 0;
        size_t newConfigs = 0;

        counter.ParseConfigAtEntry(dir, newConfigs);

        std::vector<Entry> entries = WalkDirectory(dir.fullPath);

        std::vector<std::string> paths;

        counter.WalkForPaths(entries, paths, newConfigs, ignoredFiles);

        for (size_t index = 0; index < paths.size(); index++)
        {
            fmt::print("path {}: {}\n", index, paths[index]);
        }

        REQUIRE(paths.size() == 3);
        REQUIRE(newConfigs == 1);
        REQUIRE(ignoredFiles == 3);
    }
}