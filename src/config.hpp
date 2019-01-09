#pragma once

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
};

/**
 * Parses data from a .computare.yml and stores the resulting read in this class
 * for use in counting.
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

private:
    std::map<std::string, std::shared_ptr<Language>> m_languages;

    size_t m_blockSize = 2048;

    bool m_ignoreHidden = true;

};