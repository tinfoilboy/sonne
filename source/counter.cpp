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

CountInfo Counter::Count(std::shared_ptr<Config> config)
{
    CountInfo info = {};

    std::string ext = _GetExtension(m_path);

    std::shared_ptr<Language> language = nullptr;

    if (config->HasLanguage(ext))
    {
        language = config->GetLanguage(ext);
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

    std::ifstream in(m_path);

    if (!in.good())
    {
        Fatal(fmt::format("Failed to open file at path: '{}'", m_path));
    }

    in.seekg(0, std::ios::end); // seek to the end of the buffer to grab size

    size_t bufferSize = static_cast<size_t>(in.tellg());

    std::vector<char> buffer(bufferSize);
    
    in.seekg(0); // go back to beginning of buffer

    in.read(buffer.data(), bufferSize);

    _CountFromBuffer(buffer, language, info);

    return info;
}

std::string Counter::_GetExtension(const std::string& path)
{
    auto lastPeriod = path.find_last_of('.');

    if (lastPeriod != std::string::npos)
        return path.substr(lastPeriod + 1, path.size() - (lastPeriod + 1));

    return "";
}

char Counter::_GetBufferLookahead(std::vector<char>& buffer, const size_t& index)
{
    return ((index < buffer.size()) ? buffer.at(index) : '\0');
}

void Counter::_CountFromBuffer(std::vector<char>& buffer, std::shared_ptr<Language> language, CountInfo& info)
{
    CountData data(
        0, // lineLength
        0, // lineLengthWithoutWhitespace
        0, // index
        CountState::NORMAL, // state
        true, // shouldCountBlockLine
        false, // wasBlockComment
        language,
        info,
        buffer
    );

    for (size_t index = 0; index < buffer.size(); index++)
    {
        data.index = index;

        char current = buffer.at(index); // grab the current character

        // skip past carriage return
        if (current == '\r')
        {
            continue;
        }

        if (current == '\n')
        {
            info.totalLines++;

            if (data.lineLengthWithoutWhitespace == 0)
            {
                info.emptyLines++;
            }

            _LanguageNewLineCheck(data);

            // reset both line lengths
            data.lineLength = 0;
            data.lineLengthWithoutWhitespace = 0;

            continue;
        }

        bool skip = _LanguageCommentStringChecks(data);

        if (current != ' ' && current != '\t')
        {
            data.lineLengthWithoutWhitespace++;
        }

        data.lineLength++;

        if (skip)
        {
            continue;
        }
    }

    // interpret the last line of the file as there's no \n to piggyback on
    info.totalLines++;

    _LanguageNewLineCheck(data);

    if (data.lineLengthWithoutWhitespace == 0)
    {
        info.emptyLines++; // increment the empty lines for the last line as it has no \n char
    }
}

void Counter::_LanguageNewLineCheck(CountData& data)
{
    if (data.language == nullptr)
    {
        return; // only do these operations if a language is currently set
    }

    bool shouldCountAsCode = false; // an override to count a line as code, used for block comments

    if (data.state == CountState::LINE_COMMENT || (data.state == CountState::BLOCK_COMMENT && data.shouldCountBlockLine) ||
        data.wasBlockComment)
    {
        if (data.wasBlockComment)
        {
            data.wasBlockComment = false;
            
            // force the line to count as code if the end delimiter is not the only thing on the line
            shouldCountAsCode = (data.lineLengthWithoutWhitespace != data.language->blockCommentEnd.size());        
        }

        if (!shouldCountAsCode)
        {
            data.info.commentLines++; // only add a comment line if it is a pure comment line
        }
    }

    if ((data.state == CountState::NORMAL || data.state == CountState::STRING) && data.lineLengthWithoutWhitespace > 0 || shouldCountAsCode)
    {
        data.info.codeLines++; // increment the lines of source code if we are not in a comment and if line is not empty
    }

    if (data.state == CountState::LINE_COMMENT)
    {
        data.state = CountState::NORMAL; // reset the state of the language count parser to normal text
    }

    if (!data.shouldCountBlockLine)
    {
        data.shouldCountBlockLine = true; // block comment lines should now be counted after the first line not counted
    }
}

bool Counter::_CompareStringToBuffer(std::vector<char>& buffer, std::string& compare, const size_t& start)
{
    // check if the first character matches, and if not we can already say that the string doesn't match
    if (buffer[start] != compare[0])
    {
        return false;
    }

    for (size_t i = 1; i < compare.size(); i++)
    {
        char& compareChar = compare.at(i);

        if (_GetBufferLookahead(buffer, start + i) != compareChar)
        {
            return false; // the current character is not the same of that in the buffer, so no match
        }
    }

    return true;
}

bool Counter::_CheckStringDelimiter(std::vector<char>& buffer, std::vector<std::string>& delimiters, const size_t& start)
{
    for (auto& delimiter : delimiters) // check all string delimiters for a string begin
    {
        bool hasDelimiter = _CompareStringToBuffer(buffer, delimiter, start);

        if (hasDelimiter)
        {
            return true;
        }
    }

    return false;
}

bool Counter::_LanguageCommentStringChecks(CountData& data)
{
    if (data.language == nullptr)
    {
        return false; // return out and do not skip if we do not have a language set
    }

    // we are already in a line comment, thus we can just add to the line length and ignore the rest of the chars
    if (data.state == CountState::LINE_COMMENT)
    {
        return true;
    }

    if (data.state == CountState::NORMAL)
    {
        if (!(data.language)->stringDelimiters.empty()) // check if we are about to begin a string
        {
            bool hasDelimiter = _CheckStringDelimiter(data.buffer, data.language->stringDelimiters, data.index);

            if (hasDelimiter)
            {
                data.state = CountState::STRING;
                
                return true; // skip over the delimiter for this string
            }
        }

        // only check for a line comment only if it occurs at the beginning of the line excluding whitespace
        if (data.lineLengthWithoutWhitespace == 0 && !data.language->lineComment.empty())
        {
            bool hasLineComment = _CompareStringToBuffer(data.buffer, data.language->lineComment, data.index);

            if (hasLineComment)
            {
                data.state = CountState::LINE_COMMENT;

                return true; // skip over this character as we know we are in a line comment
            }
        }

        if (!data.language->blockCommentBegin.empty())
        {
            bool hasBlockCommentBeginning = _CompareStringToBuffer(data.buffer, data.language->blockCommentBegin, data.index);

            if (hasBlockCommentBeginning)
            {
                // only count the first line of a block comment if it is at the start of the line excluding whitespace
                data.shouldCountBlockLine = (data.lineLengthWithoutWhitespace == 0);

                data.state = CountState::BLOCK_COMMENT;

                return true; // skip the rest of this characters processing
            }
        }
    }

    if (data.state == CountState::STRING && !data.language->stringDelimiters.empty()) // check if we are at the end of a string
    {
        bool hasDelimiter = _CheckStringDelimiter(data.buffer, data.language->stringDelimiters, data.index);

        if (hasDelimiter)
        {
            data.state = CountState::NORMAL;

            return true; // skip over the ending delimiter for this string
        }
    }

    // if we have endings for block comments, check if we are at a block ender if currently counting a block comment
    if (data.state == CountState::BLOCK_COMMENT && !data.language->blockCommentEnd.empty())
    {
        bool hasBlockCommentEnding = _CompareStringToBuffer(data.buffer, data.language->blockCommentEnd, data.index);

        if (hasBlockCommentEnding)
        {
            data.state = CountState::NORMAL;

            data.wasBlockComment = true;
            
            return true; // skip this character as we know it ends a block comment
        }
    }

    return false;
}