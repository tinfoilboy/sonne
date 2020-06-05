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

    // grab the running path for the test runner, used for comparing if the right paths are grabbed
    std::string runningPath = GetRunningPath();

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

        // make sure that the walked paths match that of the local test runner
        std::vector<std::string> expectedPaths = {
            fmt::format("{}{}samples{}fun.java", runningPath, Separator, Separator),
            fmt::format("{}{}samples{}test.py", runningPath, Separator, Separator),
            fmt::format("{}{}samples{}python{}program.py", runningPath, Separator, Separator, Separator)
        };

        counter.WalkForPaths(entries, paths, newConfigs, ignoredFiles);

        REQUIRE(paths.size() == 3);

        size_t pathsMatch = 0; // paths match must be the same number as expected paths to pass

        for (size_t index = 0; index < paths.size(); index++)
        {
            std::string& path = paths.at(index);

            fmt::print("Included path for {}: {}\n", index, path);

            for (size_t matcher = 0; matcher < expectedPaths.size(); matcher++)
            {
                std::string& expected = expectedPaths.at(matcher);

                if (expected == path)
                {
                    pathsMatch++;

                    break; // break out of loop to do the next path to match
                }
            }
        }

        REQUIRE(pathsMatch == expectedPaths.size());

        REQUIRE(newConfigs == 1);
        REQUIRE(ignoredFiles == 3);
    }
}