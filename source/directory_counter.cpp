#include "computare/pch.hpp"
#include "computare/directory_counter.hpp"

#include "computare/config.hpp"
#include "computare/counter.hpp"

DirectoryCounter::DirectoryCounter(const std::string& path)
    :
    m_path(path)
{
}

DirectoryInfo DirectoryCounter::Run(Config& config)
{
    fmt::print("Counting from directory...\n\n", threads);

    DirectoryInfo info = {};

    size_t ignoredFiles    = 0;
    size_t newConfigs      = 0;

    if (!fs::is_directory(m_path))
        Fatal("Passed a file in as a directory!");

    // push back a max number of worker threads for running counters on dir
    // entries and totalling up directory information
    for (size_t index = 0; index < threads; index++)
    {
        m_threads.push_back(std::thread([&]() {
            std::string file;
            std::unique_lock<std::mutex> lock(m_mutex);

            while (true)
            {
                // wait for a file to process or until the finished flag is set
                m_condition.wait(lock, [=]() {
                    return !m_fileQueue.empty() || m_finished;
                });

                if (m_finished && m_fileQueue.empty())
                {
                    break;
                }
                
                if (!m_fileQueue.empty())
                {
                    CounterQueue queue = m_fileQueue.front();

                    m_fileQueue.pop();

                    if (queue.type == CounterQueueType::DIRECTORY)
                    {
                        AddDirectoryContentToQueue(
                            config,
                            queue.path,
                            &ignoredFiles,
                            &newConfigs
                        );
                        
                        continue;
                    }
                    else
                    {
                        Counter counter(queue.path);
                        FileInfo counted = counter.Count(config);

                        // we've processed one file thus far so set counted to that
                        counted.files = 1;

                        // if this isn't the first time this language has been
                        // counted, add the current count to the totals
                        if (info.languageTotals.count(counted.language) > 0)
                        {
                            auto find = info.languageTotals.find(counted.language);

                            find->second += counted;
                        }
                        // otherwise insert the first count struct to the toals
                        else
                        {
                            info.languageTotals.insert(std::make_pair(
                                counted.language,
                                counted
                            ));
                        }

                        // tally up the totals
                        info.totals += counted;
                    }
                }
            }
        }));
    }

#ifdef __linux__
    m_fileQueue.push({
        CounterQueueType::DIRECTORY,
        m_path
    });
    m_condition.notify_one();
#else
    fs::recursive_directory_iterator itr(m_path);
    
    for (decltype(itr) end; itr != end; ++itr)
    {
        bool ignore = false;

        if (IsHidden(itr->path()) && config.GetIgnoreHidden())
        {
            if (fs::is_directory(itr->path()))
            {
                itr.disable_recursion_pending();
            }

            ignore = true;
        }

        if (fs::is_empty(itr->path()))
        {
            if (fs::is_directory(itr->path()))
            {
                itr.disable_recursion_pending();
            }

            ignore = true;
        }

        // check if anything in this path matches the ignore
        for (auto& kv : config.GetIgnored())
        {
            std::string key  = kv.first;
            std::string path = itr->path();

            // add a trailing slash to match dirs
            if (fs::is_directory(path))
                path += "/";

            // the ignore was found, skip this file/dir
            if (path.find(key) != std::string::npos)
            {
                if (fs::is_directory(itr->path()))
                {
                    itr.disable_recursion_pending();
                }

                ignore = true;

                break;
            }
        }

        if (fs::is_directory(itr->path()))
        {
            std::string configPath = fmt::format("{}/.computare.yml", itr->path().string());

            if (fs::exists(configPath))
            {
                fmt::print("loading configuration file from '{}'\n", configPath);

                config.Parse(configPath);

                newConfigs++;
            }

            ignore = true;
        }

        if (!fs::is_regular_file(itr->path()))
            ignore = true;

        if (ignore)
        {
            ignoredFiles++;

            continue;
        }

        std::lock_guard<std::mutex> lock(m_mutex);
        m_fileQueue.push(itr->path().string());

        m_condition.notify_one();
    }
#endif

    // keep busy while the files are being processed
    while (!m_fileQueue.empty() || m_currentDirectories > 0)
        asm("");

    if (newConfigs > 0)
        fmt::print("\n");

    fmt::print("ignored {} file(s)/dir(s) when processing\n", ignoredFiles);

    std::unique_lock<std::mutex> lock(m_mutex);
    m_finished = true;

    m_condition.notify_all();

    return info;
}

bool DirectoryCounter::IsHidden(const fs::path& path)
{
    const std::string name = path.filename();

    return name == "." || name == ".." || name[0] == '.';
}

void DirectoryCounter::AddDirectoryContentToQueue(
    Config& config,
    const std::string& path,
    size_t* ignoredFiles,
    size_t* newConfigs
)
{
#ifdef __linux__
    DIR*    directoryHandle;
    dirent* fileEntry;

    m_currentDirectories++;

    while ((directoryHandle = opendir(path.c_str())) == NULL)
    {
        fmt::print("could not open directory at '{}', closing", path);
        exit(-1);
    }

    bool ignore = false;

    while ((fileEntry = readdir(directoryHandle)) != NULL)
    {
        // convert entry name to std::string for ease of use
        std::string fileName = std::string(fileEntry->d_name);

        // build a string path for this entry
        std::string filePath = fmt::format(
            "{}/{}",
            path,
            fileName
        );

        // make sure that the name of the entry isn't '.' or '..'
        if (fileName == "." || fileName == "..")
            continue;
        
        // check for a computare config file name and parse if name matches
        if (fileName == ".computare.yml")
        {
            ignore = true;
        }

        // check if we have hidden files ignored and if so, check if hidden
        // if it is hidden, add to the ignored files tally and continue
        if (fileName[0] == '.' && config.GetIgnoreHidden())
        {
            (*ignoredFiles)++;

            ignore = true;
        }

        // add a trailing slash to match dirs
        if (fileEntry->d_type == DT_DIR)
        {
            filePath += "/";

            std::string potentialConfig = fmt::format("{}.computare.yml", filePath);

            // check if dir has a config file and if so parse it before moving
            if (fs::exists(potentialConfig))
            {
                fmt::print("loading configuration file from '{}'\n", potentialConfig);

                config.Parse(potentialConfig);

                (*newConfigs)++;
            }
        }

        // check if anything in this path matches the ignore
        for (auto& kv : config.GetIgnored())
        {
            std::string key = kv.first;

            // the ignore was found, skip this file/dir
            if (filePath.find(key) != std::string::npos)
            {
                (*ignoredFiles)++;

                ignore = true;

                break;
            }
        }

        if (ignore)
        {
            ignore = false;

            continue;
        }

        // remove the trailing slash for later
        if (fileEntry->d_type == DT_DIR)
           filePath.pop_back();

        if (fileEntry->d_type != DT_DIR && fileEntry->d_type != DT_REG)
            continue;

        m_fileQueue.push({
            ((fileEntry->d_type == DT_DIR) ? CounterQueueType::DIRECTORY : CounterQueueType::FILE),
            filePath
        });

        m_condition.notify_one();
    }

    closedir(directoryHandle);

    m_currentDirectories--;
#endif
}