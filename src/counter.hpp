#pragma once

struct FileInfo
{
    std::string language          = "";
    size_t      totalLines        = 0;
    size_t      emptyLines        = 0;
    size_t      codeLines         = 0;
    size_t      commentLines      = 0;
    size_t      averageLineLength = 0;
};

static inline void PrintSingleFileInfo(const FileInfo& info)
{
    fmt::print("Language: {}\n", info.language);
    fmt::print("Total Line Count: {}\n", info.totalLines);
    fmt::print("Empty Lines: {}\n", info.emptyLines);
    fmt::print("Code Lines: {}\n", info.codeLines);
    fmt::print("Comment Lines: {}\n", info.commentLines);
    fmt::print("Average Line Length: {}\n", info.averageLineLength);
}

/**
 * Class that takes in a file path and counts data about that file such as line
 * count, average line length, and language of the file based on its extension.
 */
class Counter
{
public:
    Counter(const std::string& path);

    FileInfo Count();

private:
    std::string m_path;

};