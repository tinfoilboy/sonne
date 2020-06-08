#pragma once

namespace Sonne
{

    /**
     Structure containing information on how to read a specific language using
     based on its extension. Contains information such as comment tokens.
     */
    struct Language
    {

        std::string name              = "";
        std::string lineComment       = "";
        std::string blockCommentBegin = "";
        std::string blockCommentEnd   = "";

        // a list of file extensions that this language is able to parse to
        std::vector<std::string> extensions;

        // a list of strings for defining strings in the corresponding language. used for validating comments.
        std::vector<std::string> stringDelimiters;

    };

    /**
     Parses data from a .sonne.json and stores the resulting read in this class
     for use in counting.
     
     Multiple configs can be loaded and stacked for per-project configuration
     such as custom language support or project-specific file/dir ignores.
     */
    class Config
    {

    public:

        void Parse(const std::string& path);

        void Write(std::ostream& stream);

        void Write(const std::string& path);

        bool HasLanguage(const std::string& extension) const;

        std::shared_ptr<Language> GetLanguage(const std::string& extension) const;

        inline void AddLanguage(std::shared_ptr<Language> language)
        {
            for (auto& extension : language->extensions)
            {
                this->m_languages.insert(std::make_pair(extension, language));
            }
        }

        inline void SetIgnoreHidden(bool state)
        {
            this->m_ignoreHidden = state;
        }

        inline bool GetIgnoreHidden() const
        {
            return m_ignoreHidden;
        }

        inline void AddIgnored(std::string path, bool ignore)
        {
            m_ignored.insert(std::make_pair(path, ignore));
        }

        inline std::map<std::string, std::shared_ptr<Language>>& GetLanguages()
        {
            return m_languages;
        }

        inline std::map<std::string, bool>& GetIgnored()
        {
            return m_ignored;
        }

    private:

        std::map<std::string, std::shared_ptr<Language>> m_languages;

        std::map<std::string, bool> m_ignored;

        bool m_ignoreHidden = true;

        nlohmann::json _ConstructConfigJSON();

    };

}