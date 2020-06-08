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

            language->name = languageNode["name"].get<std::string>();

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

            m_ignored[ignoreStr] = true;
        }
    }

    if (configJSON.contains("ignoreHidden"))
    {
        m_ignoreHidden = configJSON["ignoreHidden"].get<bool>();
    }
}

void Config::Write(std::ostream& stream)
{
    nlohmann::json configObject = _ConstructConfigJSON();

    stream << configObject;
}

void Config::Write(const std::string& path)
{
    nlohmann::json configObject = _ConstructConfigJSON();

    std::ofstream out(path);

    out << std::setw(4) << configObject;

    out.close();
}

bool Config::HasLanguage(const std::string& extension) const
{
    return m_languages.count(extension) > 0;
}

std::shared_ptr<Language> Config::GetLanguage(const std::string& extension) const
{
    return m_languages.at(extension);
}

nlohmann::json Config::_ConstructConfigJSON()
{
    nlohmann::json configObject;

    configObject["ignoreHidden"] = m_ignoreHidden;

    nlohmann::json languageArray = nlohmann::json::array();

    size_t index = 0;

    std::vector<std::string> alreadyInsertedLanguages;

    for (auto& language : m_languages)
    {
        nlohmann::json langObject;

        // don't add a language that we've already added to the array
        if (std::count(alreadyInsertedLanguages.begin(), alreadyInsertedLanguages.end(), language.second->name) > 0)
        {
            continue;
        }

        langObject["name"] = language.second->name;
        langObject["extensions"] = language.second->extensions;

        if (!language.second->lineComment.empty())
        {
            langObject["lineComment"] = language.second->lineComment;
        }

        if (!language.second->blockCommentBegin.empty())
        {
            langObject["blockCommentBegin"] = language.second->blockCommentBegin;
        }

        if (!language.second->blockCommentEnd.empty())
        {
            langObject["blockCommentEnd"] = language.second->blockCommentEnd;
        }

        if (!language.second->stringDelimiters.empty())
        {
            langObject["stringDelimiters"] = language.second->stringDelimiters;
        }

        languageArray[index] = langObject;

        index++;

        alreadyInsertedLanguages.push_back(language.second->name);
    }

    configObject["languages"] = languageArray;

    nlohmann::json ignoreArray = nlohmann::json::array();

    index = 0;

    for (auto& ignore : m_ignored)
    {
        ignoreArray[index] = ignore.first;

        index++;
    }

    configObject["ignore"] = ignoreArray;

    return configObject;
}