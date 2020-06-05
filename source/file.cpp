#include "computare/pch.hpp"
#include "computare/file.hpp"

using namespace computare;

Entry computare::GetFSEntry(const std::string& path)
{
    Entry entry;

#ifdef _WIN32
    WIN32_FIND_DATA data = {0};

    HANDLE fileHandle = FindFirstFile(path.c_str(), &data);

    // an entry is not valid if the file/directory could not be found, which is signaled with INVALID_HANDLE_VALUE
    entry.isValid = !(fileHandle == INVALID_HANDLE_VALUE);

    if (!entry.isValid)
    {
        fmt::print("not a valid entry at {}! code: {}\n", path, GetLastError());

        return entry;
    }

    entry.isHidden = (data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN);
    entry.isDirectory = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

    // convert the high and low parts of the file size to one size_t integer.
    ULARGE_INTEGER fileSizeLarge;
    
    fileSizeLarge.HighPart = data.nFileSizeHigh;
    fileSizeLarge.LowPart  = data.nFileSizeLow;

    entry.fileSize = static_cast<size_t>(fileSizeLarge.QuadPart);

    // grab the full path for the file to save in the entry
    char canonicalPath[MAX_PATH];

    DWORD length = GetFullPathName(path.c_str(), MAX_PATH, canonicalPath, nullptr);

    if (length <= 0)
    {
        fmt::print("Failed to get full path for file: {}", path);

        entry.isValid = false;

        return entry;
    }

    entry.fullPath = std::string(canonicalPath, length);

    FindClose(fileHandle);
#else
    stat statBuffer;

    int err = stat(path.c_str(), &statBuffer);

    if (err < 0)
    {
        fmt::print("Failed to stat at path: {}\n", path);

        entry.isValid = false;

        return entry;
    }

    entry.fileSize = static_cast<size_t>(statBuffer.st_size);

    entry.isDirectory = (S_ISDIR(statBuffer.st_mode) > 0);

    // grab the full path for the file
    char canonicalPath[PATH_MAX];

    char* pathPtr = realPath(path.c_str(), canonicalPath);

    if (pathStr == nullptr)
    {
        fmt::print("Failed to get full path for file: {}", path);

        entry.isValid = false;

        return entry;
    }

    entry.fullPath = std::string(canonicalPath);
#endif

    return entry;
}