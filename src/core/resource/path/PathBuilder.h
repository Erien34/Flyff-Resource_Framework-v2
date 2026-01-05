#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include "resource/path/FileEntry.h"
#include "resource/descriptors/Descriptor.h"

class PathBuilder
{
public:
    static std::vector<FileEntry> build(
        const std::string& resourceRoot,
        const std::vector<modules::descriptors::Descriptor>& descriptors);

    static void addFile(
        const std::filesystem::path& absPath,
        const std::string& domain,
        FileEntry::Source source);

private:
    static std::vector<FileEntry> s_entries;
};
