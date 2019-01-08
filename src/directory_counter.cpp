#include "pch.hpp"
#include "directory_counter.hpp"

#include "config.hpp"

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

                m_condition.wait(lock, [this] {
                    return this->m_finished || !this->m_fileQueue.empty();
                });

                if (m_finished && m_fileQueue.empty())
                {
                    break;
                }

                m_workingThreads++;
                
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

                m_workingThreads--;
            }
        }));
    }

    for (auto& entry : std::experimental::filesystem::recursive_directory_iterator(m_path))
    {
        m_fileQueue.push(entry.path().string());

        m_condition.notify_one();
    }

    while (true)
    {
        if (m_fileQueue.empty())
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_finished = true;

            m_condition.notify_all();

            break;
        }
    }

    return info;
}