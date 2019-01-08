#include "pch.hpp"
#include "counter.hpp"

Counter::Counter(const std::string& path)
    :
    m_path(path)
{
}

FileInfo Counter::Count()
{
    FileInfo info = {};

    std::ifstream in;
    in.open(m_path);

    if (!in.good())
    {
        Fatal("Failed to open file at path: " + m_path);
    }

    char current = '\0';

    size_t currentLineLength = 0;

    while (in.get(current))
    {
        if (current == '\n')
        {
            info.totalLines++;

            if (currentLineLength <= 0)
                info.emptyLines++;

            info.averageLineLength += currentLineLength;

            currentLineLength = 0;

            continue;
        }

        currentLineLength++;
    }

    info.averageLineLength /= info.totalLines;

    return info;
}