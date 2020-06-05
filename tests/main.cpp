#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <computare/pch.hpp>

#include <computare/file.hpp>

using namespace computare;

inline void CountSubdirectory(Entry& directory, size_t nestAmt, size_t& totalEntries)
{
    for (size_t index = 0; index < directory.children.size(); index++)
    {
        Entry entry = directory.children[index];
        
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