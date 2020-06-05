#include "computare/pch.hpp"
#include "computare/counter.hpp"

#include "computare/file.hpp"
#include "computare/config.hpp"

using namespace Computare;

Counter::Counter(const std::string& path)
    :
    m_path(path)
{
}

FileInfo Counter::Count(const Config& config)
{
    FileInfo info = {};

    std::string ext = GetExtension(m_path);

    std::shared_ptr<Language> language = nullptr;

    if (config.HasLanguage(ext))
    {
        language = config.GetLanguage(ext);
    }

    if (language != nullptr)
    {
        info.language = language->name;
    }

    Entry file = GetFSEntry(m_path);

    if (!file.isValid || file.isDirectory)
    {
        Fatal(fmt::format("Invalid file passed to counter! Path: {}\n", file.fullPath));
    }

    std::ifstream in;
    in.open(m_path);

    if (!in.good())
        Fatal("Failed to open file at path: " + m_path);

    char current = '\0';

    std::vector<char> readBlock(config.GetBlockSize());
    
    CountState        countState        = CountState::REGULAR;
    CommentCheckState checkState        = CommentCheckState::NONE;
    size_t            currentLineLength = 0;
    bool              hasComments       = false;
    std::string       lineComment       = "";
    std::string       blockCommentBegin = "";
    std::string       blockCommentEnd   = "";
    std::string       currentCheck      = "";
    bool              wasBlockComment   = false;

    if (language != nullptr)
    {
        hasComments       = true;
        lineComment       = language->lineComment;
        blockCommentBegin = language->blockCommentBegin;
        blockCommentEnd   = language->blockCommentEnd;
    }

    size_t maxCommentSize = lineComment.size();

    if (maxCommentSize < blockCommentBegin.size())
    {
        maxCommentSize = blockCommentBegin.size();
    }

    if (maxCommentSize < blockCommentEnd.size())
    {
        maxCommentSize = blockCommentEnd.size();
    }

    while (!in.eof())
    {
        in.read(readBlock.data(), config.GetBlockSize());
        
        size_t currentSize = in.gcount();

        for (size_t index = 0; index < currentSize; index++)
        {
            current = readBlock[index];

            if (checkState == CommentCheckState::CHECKING)
                currentCheck += current;

            if (current == '\n')
            {
                info.totalLines++;

                if (currentLineLength <= 0)
                    info.emptyLines++;

                if (language != nullptr && countState == CountState::REGULAR)
                    info.codeLines++;

                if (
                    language != nullptr &&
                    (
                        countState == CountState::LINE_COMMENT  ||
                        countState == CountState::BLOCK_COMMENT ||
                        wasBlockComment
                    )
                )
                {
                    if (countState == CountState::LINE_COMMENT)
                        countState = CountState::REGULAR;

                    info.commentLines++;
                }

                info.averageLineLength += currentLineLength;

                currentLineLength = 0;

                continue;
            }

            if (
                hasComments                           &&
                checkState == CommentCheckState::NONE &&
                (
                    current == lineComment[0]       ||
                    current == blockCommentBegin[0] ||
                    current == blockCommentEnd[0]
                )
            )
            {
                if (
                    lineComment.size()       == 1 ||
                    blockCommentBegin.size() == 1 ||
                    blockCommentEnd.size()   == 1
                )
                {
                    if (current == lineComment[0])
                        countState = CountState::LINE_COMMENT;
                    
                    if (current == blockCommentBegin[0])
                        countState = CountState::BLOCK_COMMENT;

                    if (current == blockCommentEnd[0])
                        countState = CountState::REGULAR;

                    currentLineLength++;

                    continue;
                }
                else
                {
                    checkState    = CommentCheckState::CHECKING;
                    currentCheck += current;
                }
            }

            if (
                checkState          == CommentCheckState::CHECKING &&
                currentCheck        == lineComment
            )
            {
                countState   = CountState::LINE_COMMENT;
                checkState   = CommentCheckState::NONE;
                currentCheck = "";
            }
            else if (
                checkState          == CommentCheckState::CHECKING &&
                currentCheck        == blockCommentBegin
            )
            {
                countState      = CountState::BLOCK_COMMENT;
                checkState      = CommentCheckState::NONE;
                currentCheck    = "";
            }
            else if (
                checkState          == CommentCheckState::CHECKING &&
                currentCheck        == blockCommentEnd
            )
            {
                countState      = CountState::REGULAR;
                checkState      = CommentCheckState::NONE;
                currentCheck    = "";
                wasBlockComment = true;
                currentLineLength++;

                continue;
            }
            
            if (hasComments && currentCheck.size() > maxCommentSize)
            {
                checkState   = CommentCheckState::NONE;
                currentCheck = "";
            }

            if (wasBlockComment)
                wasBlockComment = false;

            currentLineLength++;
        }
    }

    // add a single line for eof
    info.totalLines++;

    if (info.averageLineLength > 0)
        info.averageLineLength /= info.totalLines;

    return info;
}

std::string Counter::GetExtension(const std::string& path)
{
    auto lastPeriod = path.find_last_of('.');

    if (lastPeriod != std::string::npos)
        return path.substr(lastPeriod + 1, path.size() - (lastPeriod + 1));

    return "";
}