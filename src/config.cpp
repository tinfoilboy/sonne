#include "pch.hpp"
#include "config.hpp"

void Config::Parse(const std::string& path)
{
    YAML::Node file = YAML::LoadFile(path);

    // Load each language into the language map
    if (file["languages"])
    {
        YAML::Node languages = file["languages"];

        for (size_t index = 0; index < languages.size(); index++)
        {
            YAML::Node langNode = languages[index];
            std::shared_ptr<Language> language = std::make_shared<Language>();

            language->name = langNode["language"].as<std::string>();

            if (langNode["line-comment"])
                language->lineComment = langNode["line-comment"].as<std::string>();
            
            if (langNode["block-comment-begin"])
                language->blockCommentBegin = langNode["block-comment-begin"].as<std::string>();
            
            if (langNode["block-comment-end"])
                language->blockCommentEnd = langNode["block-comment-end"].as<std::string>();
        
            // iterate through the extensions and add each to the map with their
            // the same language struct pointer to conserve some memory
            for (size_t index = 0; index < langNode["extensions"].size(); index++)
                m_languages.insert(std::pair<std::string, std::shared_ptr<Language>>(
                    langNode["extensions"][index].as<std::string>(),
                    language
                ));
        }
    }

    if (file["block-size"])
        m_blockSize = file["block-size"].as<size_t>();

    if (file["ignore-hidden"])
        m_ignoreHidden = file["ignore-hidden"].as<bool>();
}

bool Config::HasLanguage(const std::string& extension) const
{
    return m_languages.count(extension) > 0;
}

std::shared_ptr<Language> Config::GetLanguage(const std::string& extension) const
{
    return m_languages.at(extension);
}