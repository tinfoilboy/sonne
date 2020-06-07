#pragma once

namespace Sonne
{

    struct Language;

    class Config;

    struct CountInfo
    {

        std::string language = "Plain Text"; // the language that this file is classified as

        size_t      files = 1; // the total amount of files counted

        size_t      totalLines = 0;
        size_t      emptyLines = 0;
        size_t      codeLines = 0;
        size_t      commentLines = 0;

        CountInfo& operator+=(const CountInfo& info)
        {
            this->files += info.files;
            this->totalLines += info.totalLines;
            this->emptyLines += info.emptyLines;
            this->codeLines += info.codeLines;
            this->commentLines += info.commentLines;

            return *this;
        }

    };

    enum class CountState : uint8_t
    {

        NORMAL = 0,
        LINE_COMMENT = 1,
        BLOCK_COMMENT = 2,
        STRING = 3

    };

    struct CountData
    {

        size_t lineLength = 0;
        size_t lineLengthWithoutWhitespace = 0;

        size_t index = 0;

        CountState state = CountState::NORMAL;

        bool shouldCountBlockLine = true;
        bool wasBlockComment      = false;

        std::shared_ptr<Language> language = nullptr;

        CountInfo& info;

        std::vector<char>& buffer;

        CountData(
            size_t lineLength,
            size_t lineLengthWithoutWhitespace,
            size_t index,
            CountState state,
            bool shouldCountBlockLine,
            bool wasBlockComment,
            std::shared_ptr<Language> language,
            CountInfo& info,
            std::vector<char>& buffer
        )
            :
            lineLength(lineLength),
            lineLengthWithoutWhitespace(lineLengthWithoutWhitespace),
            index(index),
            state(state),
            shouldCountBlockLine(shouldCountBlockLine),
            wasBlockComment(wasBlockComment),
            language(language),
            info(info),
            buffer(buffer)
        {
        }
    
    };

    /**
     Class that takes in a file path and counts data about that file such as line
     count, average line length, and language of the file based on its extension.
     */
    class Counter
    {

    public:

        /**
         Create a counter for the file at the current path.
         */
        Counter(const std::string& path);

        /**
         Return a FileInfo struct relating to the metrics of the countable file.
         
         Pass in a config for language support.
         */
        CountInfo Count(std::shared_ptr<Config> config);

    private:

        /*
        Path to the file that is currently being counted.
        */
        std::string m_path = "";

        /**
         Try and grab a file extension from the path specified.
         */
        std::string _GetExtension(const std::string& path);

        /**
         Grab a lookahead character for a buffer, while checking if the lookahead is valid.
         
         If an invalid lookahead is found, a null character is returned.
         */
        char _GetBufferLookahead(std::vector<char>& buffer, const size_t& index);

        /**
         Used when no language info is found for the file to be counted.

         Just counts all newlines in the file and updates the info passed in.
         */
        void _CountFromBuffer(std::vector<char>& buffer, std::shared_ptr<Language> language, CountInfo& info);

        /*
        Method for checking if any language-oriented attributes are set when a newline is found
        */
        void _LanguageNewLineCheck(CountData& data);

        /*
        Checks if a string matches with a portion of the buffer passed in.
        */
        bool _CompareStringToBuffer(std::vector<char>& buffer, std::string& compare, const size_t& start);

        /*
        Check if the buffer has a string delimiter at the current index in the buffer.
        */
        bool _CheckStringDelimiter(std::vector<char>& buffer, std::vector<std::string>& delimiters, const size_t& start);

        /*
        Method for setting state based on config comment and string definitions.

        Line length is used to figure out if a line comment is at the beginning of a line.
        
        Returns true if the loop should skip ahead after running this method.
        */
        bool _LanguageCommentStringChecks(CountData& data);

    };
}