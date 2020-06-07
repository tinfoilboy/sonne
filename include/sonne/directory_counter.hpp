#pragma once

#include "sonne/counter.hpp"

namespace Sonne
{

    class Config;

    struct Entry;

    struct DirectoryInfo
    {

        std::map<std::string, CountInfo> totals;
    
    };

    /**
     Takes in a path to a directory and tries to walk that directory, counting
     each file that it finds for lines.
     */
    class DirectoryCounter
    {

    public:

        DirectoryCounter(const std::string& path, std::shared_ptr<Config> config);

        DirectoryInfo Run();

        /**
         Walk through entries to grab paths to append to the second vector.

         The second vector is what will actually be used by the class to count files.
         */
        void WalkForPaths(
            std::vector<Entry>& entries,
            std::vector<std::string>& paths,
            size_t& configs,
            size_t& ignored);

        /**
         Attempt to parse a config file within the directory entry passed in.
         */
        void ParseConfigAtEntry(Entry& entry, size_t& configs);

    private:

        std::string m_path;

        std::shared_ptr<Config> m_config;

    };
}