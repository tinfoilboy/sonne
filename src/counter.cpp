#include "pch.hpp"
#include "counter.hpp"

#include "config.hpp"

Counter::Counter(const std::string& path)
    :
    m_path(path)
{
}

FileInfo Counter::Count(Config& config)
{
    FileInfo info = {};

    std::string ext = GetExtension(m_path);

    std::shared_ptr<Language> language = nullptr;

    if (config.HasLanguage(ext))
        language = config.GetLanguage(ext);

    if (language != nullptr)
    {
        info.language = language->name;
        fmt::print("File language: {}\n", language->name);
    }

    std::ifstream in;
    in.open(m_path);

    if (!in.good())
    {
        Fatal("Failed to open file at path: " + m_path);
    }

    char current = '\0';

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

    size_t maxCommentSize = std::max({
        lineComment.size(),
        blockCommentBegin.size(),
        blockCommentEnd.size()
    });

    while (in.get(current))
    {
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
            checkState    = CommentCheckState::CHECKING;
            currentCheck += current;
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