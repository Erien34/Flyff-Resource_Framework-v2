#include "PathBuilder.h"
#include <filesystem>

namespace fs = std::filesystem;

std::vector<FileEntry> PathBuilder::build(
    const std::string& resourceRoot,
    const std::vector<modules::descriptors::Descriptor>& descriptors)
{
    std::vector<FileEntry> out;

    if (!fs::exists(resourceRoot))
        return out;

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

                    out.push_back(std::move(fe));
                }
            }
        }
    }

    return out;
}
