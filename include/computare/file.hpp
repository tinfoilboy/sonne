#pragma once

namespace computare {
    
    struct Entry {

        bool isValid = true;

        bool isDirectory = false;

        /**
         * this attribute really only matters on Windows, as hidden files are an actual attribute rather than a dotfile
         */
        bool isHidden    = false;

        std::string fullPath = "";

        size_t fileSize = 0;

    };

    /**
     * Grab an entry from the filesystem, with information about that entry.
     */
    Entry GetFSEntry(const std::string& path);

    /**
     * Gets a vector of each entry in a directory.
     */
    //std::vector<Entry> ListDirectory(const std::string& path);

};