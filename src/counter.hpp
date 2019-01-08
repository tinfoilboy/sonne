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
    std::cout << "Language: "
              << termcolor::bold
              << info.language
              << termcolor::reset
              << std::endl;

    std::cout << "Total Line Count: "
              << termcolor::bold
              << info.totalLines
              << termcolor::reset
              << std::endl;

    std::cout << "Empty Lines: "
              << termcolor::bold
              << info.emptyLines
              << termcolor::reset
              << std::endl;
    
    std::cout << "Code Lines: "
              << termcolor::bold
              << info.codeLines
              << termcolor::reset
              << std::endl;

    std::cout << "Comment Lines: "
              << termcolor::bold
              << info.commentLines
              << termcolor::reset
              << std::endl;

    std::cout << "Average Line Length: "
              << termcolor::bold
              << info.averageLineLength
              << termcolor::reset
              << std::endl;
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