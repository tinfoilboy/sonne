#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <sonne/pch.hpp>

#include <sonne/file.hpp>
#include <sonne/config.hpp>
#include <sonne/config_generator.hpp>
#include <sonne/directory_counter.hpp>

using namespace Sonne;

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
        REQUIRE(entry.fileSize == 262);
    }

    SECTION("filesystem walk works")
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

TEST_CASE("counter works properly")
{
    std::shared_ptr<Config> config = GenerateDefaultConfig();

    SECTION("count plain text reports correctly")
    {
        Counter counter("samples/test.txt");

        CountInfo info = counter.Count(config);

        REQUIRE(info.language == "Plain Text");
        REQUIRE(info.files == 1);
        REQUIRE(info.totalLines == 5);
        REQUIRE(info.emptyLines == 1);
    }

    SECTION("count code reports correctly for test.cpp")
    {
        Counter counter("samples/test.cpp");

        CountInfo info = counter.Count(config);

        REQUIRE(info.language == "C/C++ Source");
        REQUIRE(info.files == 1);
        REQUIRE(info.totalLines == 22);
        REQUIRE(info.codeLines == 10);
        REQUIRE(info.emptyLines == 5);
        REQUIRE(info.commentLines == 7);
    }

    SECTION("count code reports correctly for test.java")
    {
        Counter counter("samples/test.java");

        CountInfo info = counter.Count(config);

        REQUIRE(info.language == "Java");
        REQUIRE(info.files == 1);
        REQUIRE(info.totalLines == 23);
        REQUIRE(info.codeLines == 10);
        REQUIRE(info.emptyLines == 4);
        REQUIRE(info.commentLines == 9);
    }

    SECTION("count code reports correctly for test.lua")
    {
        Counter counter("samples/test.lua");

        CountInfo info = counter.Count(config);

        REQUIRE(info.language == "Lua");
        REQUIRE(info.files == 1);
        REQUIRE(info.totalLines == 13);
        REQUIRE(info.codeLines == 4);
        REQUIRE(info.emptyLines == 3);
        REQUIRE(info.commentLines == 6);
    }
}

TEST_CASE("directory counter works properly")
{
    // separate the previous case with two newlines
    fmt::print("\n\n");

    // grab the running path for the test runner, used for comparing if the right paths are grabbed
    std::string runningPath = GetRunningPath();

    std::shared_ptr<Config> config = GenerateDefaultConfig();

    SECTION("path walking returns correct entries")
    {
        Entry dir = GetFSEntry("ignore_test");

        DirectoryCounter counter(dir.fullPath, config);

        size_t ignoredPaths = 0;
        size_t newConfigs   = 0;

        counter.ParseConfigAtEntry(dir, newConfigs);

        std::vector<std::string> paths;

        // make sure that the walked paths match that of the local test runner
        std::vector<std::string> expectedPaths = {
            fmt::format("{}{}ignore_test{}queen.py", runningPath, Separator, Separator),
            fmt::format("{}{}ignore_test{}afile.cpp", runningPath, Separator, Separator)
        };

        std::vector<Entry> entries = WalkDirectory(dir.fullPath);

        counter.WalkForPaths(entries, paths, newConfigs, ignoredPaths);

        REQUIRE(paths.size() == 2); // we should only have two paths, 'afile.cpp' and 'queen.py'

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
        REQUIRE(ignoredPaths == 3); // ignoring both files in 'ignore' as well as 'ignored.java'
    }

    config = GenerateDefaultConfig(); // regenerate config to reset ignores

    SECTION("directory counter gets correct counts for files")
    {
        DirectoryCounter counter("samples", config);

        DirectoryInfo info = counter.Run();

        DirectoryInfo expected = {};

        expected.totals = std::map<std::string, CountInfo>();

        expected.totals.insert(std::make_pair<std::string, CountInfo>("C/C++ Source", {
            "C/C++ Source",
            1, // files
            22, // total lines
            5, // empty lines
            10, // code lines
            7 // comment lines
        }));
        
        expected.totals.insert(std::make_pair<std::string, CountInfo>("C/C++ Header", {
            "C/C++ Header",
            1, // files
            10, // total lines
            2, // empty lines
            2, // code lines
            6 // comment lines
        }));
        
        expected.totals.insert(std::make_pair<std::string, CountInfo>("Java", {
            "Java",
            1, // files
            23, // total lines
            4, // empty lines
            10, // code lines
            9 // comment lines
        }));

        expected.totals.insert(std::make_pair<std::string, CountInfo>("Lua", {
            "Lua",
            1, // files
            13, // total lines
            3, // empty lines
            4, // code lines
            6 // comment lines
        }));
        
        expected.totals.insert(std::make_pair<std::string, CountInfo>("Python", {
            "Python",
            1, // files
            11, // total lines
            3, // empty lines
            6, // code lines
            2 // comment lines
        }));

        expected.totals.insert(std::make_pair<std::string, CountInfo>("Plain Text", {
            "Plain Text",
            1, // files
            5, // total lines
            1, // empty lines
            0, // code lines
            0 // comment lines
        }));

        expected.totals.insert(std::make_pair<std::string, CountInfo>("Totals", {
            "Totals",
            6, // files
            84, // total lines
            18, // empty lines
            32, // code lines
            30 // comment lines
        }));

        bool entryNotFound = false;

        // iterate through each language returned by the counter (including the totals) and compare with the map
        // of expected info values for each language
        for (auto entry : info.totals)
        {
            REQUIRE(expected.totals.count(entry.first) > 0); // make sure that we have the expected value in the totals
            
            CountInfo& expectInfo = expected.totals.at(entry.first);

            REQUIRE(expectInfo.language == entry.second.language);
            REQUIRE(expectInfo.files == entry.second.files);
            REQUIRE(expectInfo.totalLines == entry.second.totalLines);
            REQUIRE(expectInfo.emptyLines == entry.second.emptyLines);
            REQUIRE(expectInfo.codeLines == entry.second.codeLines);
            REQUIRE(expectInfo.commentLines == entry.second.commentLines);
        }
    }
}