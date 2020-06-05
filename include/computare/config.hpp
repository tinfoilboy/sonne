#pragma once

namespace Computare
{

    /**
     * Structure containing information on how to read a specific language using
     * based on its extension. Contains information such as comment tokens.
     */
    struct Language
    {

        std::string name              = "";
        std::string lineComment       = "";
        std::string blockCommentBegin = "";
        std::string blockCommentEnd   = "";

        // a list of strings for defining strings in the corresponding language. used for validating comments.
        std::vector<std::string> stringDelimiters;

    };

    /**
     * Parses data from a .computare.yml and stores the resulting read in this class
     * for use in counting.
     *
     * Multiple configs can be loaded and stacked for per-project configuration
     * such as custom language support or project-specific file/dir ignores.
     */
    class Config
    {

    public:

        void Parse(const std::string& path);

        bool HasLanguage(const std::string& extension) const;

        std::shared_ptr<Language> GetLanguage(const std::string& extension) const;

        inline void SetIgnoreHidden(bool state)
        {
            this->m_ignoreHidden = state;
        }

        inline bool GetIgnoreHidden() const
        {
            return m_ignoreHidden;
        }

        inline void SetBlockSize(size_t size)
        {
            this->m_blockSize = size;
        }

        inline size_t GetBlockSize() const
        {
            return m_blockSize;
        }

        inline std::map<std::string, bool> GetIgnored()
        {
            return m_ignored;
        }

    private:

        std::map<std::string, std::shared_ptr<Language>> m_languages;

        std::map<std::string, bool> m_ignored;

        size_t m_blockSize = 4096;

        bool m_ignoreHidden = true;

    };

}