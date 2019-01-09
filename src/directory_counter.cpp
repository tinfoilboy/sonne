#include "pch.hpp"
#include "directory_counter.hpp"

#include "config.hpp"

namespace fs = std::experimental::filesystem;

DirectoryCounter::DirectoryCounter(const std::string& path)
    :
    m_path(path)
{
}

DirectoryCounter::~DirectoryCounter()
{
    for (auto& thread : m_threads)
        thread.join();
}

DirectoryInfo DirectoryCounter::Run(const Config& config)
{
    size_t threads = std::thread::hardware_concurrency();

    fmt::print("counting from directory with {} threads!\n", threads);

    DirectoryInfo info = {};

    // push back a max number of worker threads for running counters on dir
    // entries and totalling up directory information
    for (size_t index = 0; index < threads; index++)
    {
        m_threads.push_back(std::thread([&]() {
            std::string file;

            while (true)
            {
                std::unique_lock<std::mutex> lock(m_mutex);

                // wait for a file to process or until the finished flag is set
                m_condition.wait(lock, [this]() {
                    return m_finished || !m_fileQueue.empty();
                });

                if (m_finished && m_fileQueue.empty())
                {
                    break;
                }
                
                if (!m_fileQueue.empty())
                {
                    std::string file = m_fileQueue.front();
                        
                    m_fileQueue.pop();

                    Counter counter(file);
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
        }));
    }

    fs::recursive_directory_iterator itr(m_path);

    for (decltype(itr) end; itr != end; ++itr)
    {
        if (IsHidden(itr->path()) && config.GetIgnoreHidden())
        {
            if (fs::is_directory(itr->path()))
            {
                itr.disable_recursion_pending();
            }

            continue;
        }

        if (fs::is_empty(itr->path()))
            continue;

        if (!fs::is_regular_file(itr->path()))
            continue;

        std::lock_guard<std::mutex> lock(m_mutex);
        m_fileQueue.push(itr->path().string());

        m_condition.notify_one();
    }

    // keep busy while the files are being processed
    while (!m_fileQueue.empty())
        asm("");

    std::lock_guard<std::mutex> lock(m_mutex);
    m_finished = true;

    m_condition.notify_all();

    return info;
}

bool DirectoryCounter::IsHidden(const fs::path& path)
{
    const std::string name = path.filename();

    return name == "." || name == ".." || name[0] == '.';
}