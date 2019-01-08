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

    bool HasLanguage(const std::string& extension);

    std::shared_ptr<Language> GetLanguage(const std::string& extension);

private:
    std::map<std::string, std::shared_ptr<Language>> m_languages;

};