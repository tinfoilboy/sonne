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

                    counted.files = 1;

                    if (info.languageTotals.count(counted.language) > 0)
                    {
                        auto find = info.languageTotals.find(counted.language);

                        find->second += counted;
                    }
                    else
                    {
                        info.languageTotals.insert(std::make_pair(
                            counted.language,
                            counted
                        ));
                    }

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

    while (!m_fileQueue.empty())
    {
        m_condition.notify_one();
    }

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