#include "sonne/pch.hpp"
#include "sonne/directory_counter.hpp"

#include "sonne/file.hpp"
#include "sonne/config.hpp"

using namespace Sonne;

DirectoryCounter::DirectoryCounter(const std::string& path, std::shared_ptr<Config> config)
    :
    m_path(path),
    m_config(config)
{
}

DirectoryInfo DirectoryCounter::Run()
{
    DirectoryInfo info = {};

    // create a file info struct for the counts of the totals
    CountInfo total = {};
    total.language  = "Totals";

    total.files = 0; // make sure the total doesn't start with any file counts

    size_t ignoredFiles = 0; // the amount of files/directories ignored by the user configs
    size_t newConfigs   = 0; // the amount of new configs loaded as the directory was walked

    Entry dir = GetFSEntry(m_path);

    if (!dir.isDirectory)
    {
        Fatal("Path to count from must be directory!");
    }

    m_path = dir.fullPath; // set the base path to the full canonical path

    ParseConfigAtEntry(dir, newConfigs); // attempt to parse a config at the root before walking paths

    std::vector<Entry> entries = WalkDirectory(m_path);
    
    std::vector<std::string> paths;

    WalkForPaths(entries, paths, newConfigs, ignoredFiles);
    
    std::vector<std::future<CountInfo>> countFutures;

    for (size_t index = 0; index < paths.size(); index++)
    {
        std::string& path = paths.at(index);

        Counter counter(path);

        countFutures.push_back(std::async(std::launch::async, &Counter::Count, counter, m_config));
    }

    // go through each async future that is currently in the vector and try and tally the counts together
    //
    // main thread acts as a tally for all the counters on the other async threads.
    while (!countFutures.empty())
    {
        auto& future = countFutures.front();

        // little hack to check if future is ready immediately rather than in the future
        if (future.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready && future.valid())
        {
            std::swap(countFutures.front(), countFutures.back()); // swap the beginning with the end to get a new entry and continue

            continue;
        }

        CountInfo count = future.get();

        // if this language has occurred before in the counter, just append to the counts for that language
        if (info.totals.count(count.language) > 0)
        {
            auto find = info.totals.find(count.language);

            find->second += count;
        }
        // otherwise insert the first count struct to the totals
        else
        {
            info.totals.insert(std::make_pair(
                count.language,
                count
            ));
        }

        // tally up the totals
        total += count;

        std::swap(future, countFutures.back());
        countFutures.pop_back();
    }

    info.totals.insert(std::make_pair("Totals", total));

    return info;
}

void DirectoryCounter::WalkForPaths(
    std::vector<Entry>& entries,
    std::vector<std::string>& paths,
    size_t& configs,
    size_t& ignored)
{
    for (size_t index = 0; index < entries.size(); index++)
    {
        Entry& entry = entries.at(index);

        // ignore by default if hidden and ignoring hidden
        bool ignore = (entry.isHidden && m_config->GetIgnoreHidden());

        // skip a config file if it comes up
        if (entry.fileName == ".sonne.json")
        {
            continue;
        }

        // check if this path is in the ignore path, and if so continue
        for (auto& kv : m_config->GetIgnored())
        {
            std::string key = kv.first;

            if (key.find('/') != std::string::npos && Separator == '\\')
            {
                std::replace(key.begin(), key.end(), '/', '\\'); // replace forward slashes with back slashes for ignore
            }
            else if (key.find('\\') != std::string::npos && Separator == '/')
            {
                std::replace(key.begin(), key.end(), '\\', '/'); // replace back slashes with forward slashes for ignore
            }

            // grab a substring of the file path without the root path for matching
            std::string relativePath = entry.fullPath.substr(m_path.size() + 1, entry.fullPath.size() - m_path.size());

            size_t find = relativePath.find(key);

            // the ignore was found, skip this file/dir if the ignore directive is true
            if (find != std::string::npos)
            {
                ignored++;

                ignore = true;

                break;
            }
        }

        if (ignore)
        {
            continue; // contine if the file or directory is ignored
        }

        // if this is a directory, check if there is a config to load, then recurse
        if (entry.isDirectory && !entry.children.empty())
        {
            ParseConfigAtEntry(entry, configs);

            WalkForPaths(entry.children, paths, configs, ignored);
            
            continue; // continue as we cannot count a directory
        }
        else if (!entry.isDirectory)
        {
            // we've already accounted for directories, so this has to be a file to count from
            paths.push_back(entry.fullPath);
        }
    }
}

void DirectoryCounter::ParseConfigAtEntry(Entry& entry, size_t& configs)
{
    // if this is a directory, check if there is a config to load, then recurse
    if (!entry.isDirectory)
    {
        return;
    }

    std::string potentialConfigPath = fmt::format("{}/.sonne.json", entry.fullPath);

    Entry potentialConfig = GetFSEntry(potentialConfigPath);

    if (potentialConfig.isValid)
    {
        // todo: instead of just parsing the config into the same config, maybe create a new config deriving from both?
        // that way, you could have configs be scoped rather than just modifying the one config that you use

        m_config->Parse(potentialConfig.fullPath);

        configs++;
    }
}