#include "PathBuilder.h"
#include <filesystem>

namespace fs = std::filesystem;

std::vector<FileEntry> PathBuilder::s_entries;

void PathBuilder::addFile(
    const std::filesystem::path& absPath,
    const std::string& domain,
    FileEntry::Source source)
{
    if (!fs::exists(absPath))
        return;

    FileEntry fe;
    fe.moduleId     = "";
    fe.filename     = absPath.filename().string();
    fe.domain       = "world";
    fe.absolutePath = absPath.string();
    fe.source       = FileEntry::Source::Resource;

    s_entries.push_back(std::move(fe));
}

std::vector<FileEntry> PathBuilder::build(
    const std::string& resourceRoot,
    const std::vector<modules::descriptors::Descriptor>& descriptors)
{
    std::vector<FileEntry> out;

    if (!fs::exists(resourceRoot))
        return out;

    // bestehende Descriptor-Logik (unverändert)
    for (const auto& desc : descriptors)
    {
        for (const auto& rf : desc.resourceFiles)
        {
            for (const auto& p : fs::recursive_directory_iterator(resourceRoot))
            {
                if (!p.is_regular_file())
                    continue;

                if (p.path().filename().string() == rf.file)
                {
                    FileEntry fe;
                    fe.moduleId      = desc.id;
                    fe.filename      = rf.file;
                    fe.domain        = rf.domain;
                    fe.absolutePath  = p.path().string();
                    fe.source        = FileEntry::Source::Resource;

                    out.push_back(std::move(fe));
                }
            }
        }
    }

    // NEU: World-Dateien anhängen
    out.insert(out.end(),
               s_entries.begin(),
               s_entries.end());

    return out;
}
