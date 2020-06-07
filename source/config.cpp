#include "sonne/pch.hpp"
#include "sonne/config.hpp"

using namespace Sonne;

void Config::Parse(const std::string& path)
{
    nlohmann::json configJSON;

    std::ifstream in(path);

    if (!in.good())
    {
        Fatal("Failed to open config file!\n");
    }

    in >> configJSON;

    // Load each language into the language map
    if (configJSON.contains("languages"))
    {
        nlohmann::json languagesArray = configJSON["languages"];

        if (!languagesArray.is_array())
        {
            Fatal("Languages JSON element in config must be an array!\n");
        }

        for (size_t index = 0; index < languagesArray.size(); index++)
        {
            nlohmann::json& languageNode = languagesArray.at(index);

            std::shared_ptr<Language> language = std::make_shared<Language>();

            language->name = languageNode["language"].get<std::string>();

            if (languageNode.contains("lineComment"))
            {
                language->lineComment = languageNode["lineComment"].get<std::string>();
            }
            
            if (languageNode.contains("blockCommentBegin"))
            {
                language->blockCommentBegin = languageNode["blockCommentBegin"].get<std::string>();
            }
            
            if (languageNode.contains("blockCommentEnd"))
            {
                language->blockCommentEnd = languageNode["blockCommentEnd"].get<std::string>();
            }

            if (languageNode.contains("stringDelimiters"))
            {
                nlohmann::json stringDelimiters = languageNode["stringDelimiters"];

                for (auto& node : stringDelimiters)
                {
                    language->stringDelimiters.push_back(node.get<std::string>());
                }
            }

            nlohmann::json extensions = languageNode["extensions"];

            for (auto& node : extensions)
            {
                std::string extension = node.get<std::string>();

                language->extensions.push_back(extension);

                m_languages.insert(std::pair<std::string, std::shared_ptr<Language>>(extension, language));
            }
        }
    }

    if (configJSON.contains("ignore"))
    {
        nlohmann::json ignored = configJSON["ignore"];

        for (auto& node : ignored)
        {
            std::string ignoreStr = node.get<std::string>();

            // if the ignore string starts with an exclamation point, do not ignore that file/folder
            if (ignoreStr[0] == '!')
            {
                ignoreStr = ignoreStr.substr(1);

                m_ignored[ignoreStr] = false;
            }
            else
            {
                m_ignored[ignoreStr] = true;
            }
        }
    }

    if (configJSON.contains("ignoreHidden"))
    {
        m_ignoreHidden = configJSON["ignoreHidden"].get<bool>();
    }
}

bool Config::HasLanguage(const std::string& extension) const
{
    return m_languages.count(extension) > 0;
}

std::shared_ptr<Language> Config::GetLanguage(const std::string& extension) const
{
    return m_languages.at(extension);
}