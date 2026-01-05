#pragma once
#include <string>

struct FileEntry
{
    std::string moduleId;     // leer für World-Dateien
    std::string filename;
    std::string domain;       // z.B. "world"
    std::string absolutePath;

    enum class Source
    {
        Unknown,
        Client,
        Resource
    };

    Source source = Source::Unknown; // Client / Resource / Unknown
};
