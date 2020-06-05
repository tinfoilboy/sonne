#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <computare/pch.hpp>

#include <computare/file.hpp>

TEST_CASE("filesystem custom functions work correctly")
{
    SECTION("individual entry grabbing works correctly")
    {
        computare::Entry entry = computare::GetFSEntry("samples/test.py");

        REQUIRE(entry.isValid == true);

        // file size of the test.py file (unless updated) should always be 142
        REQUIRE(entry.fileSize == 142);
    }
}