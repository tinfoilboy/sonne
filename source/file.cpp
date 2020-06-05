#include "computare/pch.hpp"
#include "computare/file.hpp"

using namespace Computare;

#ifdef _WIN32
void Computare::SetEntryFromHandle(
    HANDLE file,
    const std::string& path,
    Entry& entry,
    WIN32_FIND_DATA& data,
    bool grabFullPath)
{
    entry.isHidden    = (data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN);
    entry.isDirectory = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

    // convert the high and low parts of the file size to one size_t integer.
    ULARGE_INTEGER fileSizeLarge;

    fileSizeLarge.HighPart = data.nFileSizeHigh;
    fileSizeLarge.LowPart = data.nFileSizeLow;

    entry.fileSize = static_cast<size_t>(fileSizeLarge.QuadPart);

    // grab the full path for the file to save in the entry
    if (grabFullPath)
    {
        char canonicalPath[MAX_PATH];

        DWORD length = GetFullPathName(path.c_str(), MAX_PATH, canonicalPath, nullptr);

        if (length <= 0)
        {
            fmt::print("Failed to get full path for file: {}", path);

            entry.isValid = false;

            return;
        }

        entry.fullPath = std::string(canonicalPath, length);
    }
}
#endif

std::string Computare::GetRunningPath()
{
    std::string path = "";

#ifdef _WIN32
    char pathBuffer[MAX_PATH];

    int size = GetModuleFileName(NULL, pathBuffer, MAX_PATH);

    // if we have anything in the buffer, copy to string and return
    if (size >= 1)
    {
        path = std::string(pathBuffer);
    }
#else
    char pathBuffer[PATH_MAX];

    int size = readlink("/proc/self/exe", pathBuffer, PATH_MAX);

    if (size >= 0)
    {
        pathBuffer[size] = '\0';
    
        path = std::string(pathBuffer);
    }
#endif

    // cut off the executable name from the path
    size_t last = path.find_last_of(Separator);

    path = path.substr(0, last);

    return path;
}

Entry Computare::GetFSEntry(const std::string& path, bool shouldClose)
{
    Entry entry;

#ifdef _WIN32
    WIN32_FIND_DATA data = {0};

    HANDLE file = FindFirstFile(path.c_str(), &data);
    
    entry.windowsHandle = file;

    // an entry is not valid if the file/directory could not be found, which is signaled with INVALID_HANDLE_VALUE
    entry.isValid = !(entry.windowsHandle == INVALID_HANDLE_VALUE);

    if (!entry.isValid)
    {
        return entry;
    }

    SetEntryFromHandle(file, path, entry, data);

    if (shouldClose)
    {
        FindClose(entry.windowsHandle);
    }
#else
    struct stat statBuffer;

    int err = stat(path.c_str(), &statBuffer);

    if (err < 0)
    {
        entry.isValid = false;

        return entry;
    }

    entry.fileSize = static_cast<size_t>(statBuffer.st_size);

    entry.isDirectory = (S_ISDIR(statBuffer.st_mode) > 0);

    // grab the full path for the file
    char canonicalPath[PATH_MAX];

    char* pathPtr = realpath(path.c_str(), canonicalPath);

    if (pathPtr == nullptr)
    {
        fmt::print("Failed to get full path for file: {}", path);

        entry.isValid = false;

        return entry;
    }

    entry.fullPath = std::string(canonicalPath);

    // if close is disabled, open the dir through dirent and set the dir handle in the entry
    if (!shouldClose)
    {
        DIR* directoryHandle;

        if ((directoryHandle = opendir(entry.fullPath.c_str())) == NULL)
        {
            fmt::print("Failed to open directory at path: {}\n", entry.fullPath);

            entry.isValid = false;

            return entry;
        }

        entry.direntHandle = directoryHandle;
    }
#endif

    return entry;
}

Entry Computare::GetNextEntry(const std::string& rootDir, Entry& previous)
{
    Entry next;

#ifdef _WIN32
    WIN32_FIND_DATA data = { 0 };

    BOOL end = FindNextFile(previous.windowsHandle, &data);

    next.windowsHandle = previous.windowsHandle;

    // if we have no more files to find, return with the findEnd flag set
    if (end == 0)
    {
        next.isValid = false;
        next.findEnd = true;

        FindClose(next.windowsHandle);

        return next;
    }

    SetEntryFromHandle(next.windowsHandle, rootDir, next, data, false);
    
    // set the entry to be a special directory for either same folder or up one directory
    if ((strcmp(data.cFileName, ".") == 0) || (strcmp(data.cFileName, "..") == 0))
    {
        next.isSpecialDirectory = true;
    }

    next.fileName = std::string(data.cFileName);

    // concatenate the full path to be the root dir and the file name
    next.fullPath = rootDir + Separator + next.fileName;

    // append a separator to the file name if this is a directory
    if (next.isDirectory)
    {
        next.fileName += Separator;
    }
#else
    dirent* direntEntry = readdir(previous.direntHandle);

    next.direntHandle = previous.direntHandle;

    if (direntEntry == NULL)
    {
        next.isValid = false;
        next.findEnd = true;

        closedir(next.direntHandle);

        return next;
    }

    // set the entry to be a special directory for either same folder or up one directory
    if ((strcmp(direntEntry->d_name, ".") == 0) || (strcmp(direntEntry->d_name, "..") == 0))
    {
        next.isSpecialDirectory = true;

        return next;
    }

    // check if the first character is a dot for the file name, and if so, set hidden to true
    if (direntEntry->d_name[0] == '.')
    {
        next.isHidden = true;
    }

    if (direntEntry->d_type == DT_DIR)
    {
        next.isDirectory = true;
    }

    next.fileName = std::string(direntEntry->d_name);

    next.fullPath = rootDir + Separator + next.fileName;

    // append a separator to the file name if this is a directory
    if (next.isDirectory)
    {
        next.fileName += Separator;
    }
#endif

    return next;
}

std::vector<Entry> Computare::WalkDirectory(const std::string& path)
{
    std::vector<Entry> entries;

    // start by grabbing the path that is specified and making sure it is a directory
    Entry root = GetFSEntry(path);

    if (!root.isValid || !root.isDirectory)
    {
        fmt::print("Not a valid directory to list from! Path: {}", path);

#ifdef _WIN32
        FindClose(root.windowsHandle); // close the file find if this was not a directory
#endif

        return entries;
    }

#ifdef _WIN32
    Entry first = GetFSEntry(path + "\\*.*", false); // append a wildcard to the end on windows
#else
    Entry first = GetFSEntry(path, false);
#endif

    while (true)
    {
        Entry next = GetNextEntry(root.fullPath, ((entries.size() == 0) ? first : entries.back()));

        if (next.findEnd)
        {
            break;
        }

        if (next.isSpecialDirectory)
        {
            continue;
        }

        // add child files and directories if this entry itself is a directory
        if (next.isDirectory)
        {
            next.children = WalkDirectory(next.fullPath);
        }

        entries.push_back(next);
    }

    return entries;
}
