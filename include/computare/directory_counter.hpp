#pragma once

#include "computare/counter.hpp"

namespace Computare
{

    class Config;

    struct Entry;

    struct DirectoryInfo
    {

        std::map<std::string, FileInfo> totals;
    
    };

    /*inline void PrintDirectoryInfo(const DirectoryInfo& info)
    {
        // the largest cell thus far is the comment lines cell, which is 15 chars
        // including the end space
        size_t largestWidth = 14;
        size_t totalWidth   = 7;

        // add one to each width for the trailing space
        for (auto& itr : info.totals)
        {
            largestWidth = std::max({
                largestWidth,
                totalWidth,
                itr.second.language.size(),
                std::to_string(itr.second.files).size(),
                std::to_string(itr.second.emptyLines).size(),
                std::to_string(itr.second.codeLines).size(),
                std::to_string(itr.second.commentLines).size(),
                std::to_string(itr.second.totalLines).size()
            });
        }

        fmt::print(
            "| {}{}| {}{}| {}{}| {}{}| {}{}| {}{}|\n",
            "Language",
            std::string(largestWidth - std::string("Language").size(), ' '),
            "Total Files",
            std::string(largestWidth - std::string("Total Files").size(), ' '),
            "Empty Lines",
            std::string(largestWidth - std::string("Empty Lines").size(), ' '),
            "Code Lines",
            std::string(largestWidth - std::string("Code Lines").size(), ' '),
            "Comment Lines",
            std::string(largestWidth - std::string("Comment Lines").size(), ' '),
            "Total Lines",
            std::string(largestWidth - std::string("Total Lines").size(), ' ')
        );

        for (auto& itr : info.totals)
        {
            fmt::print(
                "| {}{}| {}{}| {}{}| {}{}| {}{}| {}{}|\n",
                itr.second.language,
                std::string(largestWidth - itr.second.language.size(), ' '),
                itr.second.files,
                std::string(largestWidth - std::to_string(itr.second.files).size(), ' '),
                itr.second.emptyLines,
                std::string(largestWidth - std::to_string(itr.second.emptyLines).size(), ' '),
                itr.second.codeLines,
                std::string(largestWidth - std::to_string(itr.second.codeLines).size(), ' '),
                itr.second.commentLines,
                std::string(largestWidth - std::to_string(itr.second.commentLines).size(), ' '),
                itr.second.totalLines,
                std::string(largestWidth - std::to_string(itr.second.totalLines).size(), ' ')
            );
        }
    }*/

    /**
     * Takes in a path to a directory and tries to walk that directory, counting
     * each file that it finds for lines.
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