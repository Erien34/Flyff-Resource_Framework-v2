#pragma once
#include <string>

struct FileEntry
{
    std::string moduleId;      // z.B. "items"
    std::string filename;      // z.B. "propItem.txt"
    std::string absolutePath;  // D:/Flyff/Resource/propItem.txt
    std::string domain;
};
