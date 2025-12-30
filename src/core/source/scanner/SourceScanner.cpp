#include "SourceScanner.h"

#include <filesystem>

namespace fs = std::filesystem;
using namespace core::source;

SourceScanner::SourceScanner(Settings settings)
    : m_settings(std::move(settings))
{
}

void SourceScanner::scan(data::source::SourceIndexList& outIndex)
{
    outIndex.clear();

    if (!fs::exists(m_settings.sourceRoot))
        return;

    for (auto& entry : fs::recursive_directory_iterator(m_settings.sourceRoot))
    {
        if (!entry.is_regular_file())
            continue;

        const fs::path& absPath = entry.path();
        const std::string ext = absPath.extension().string();

        if (m_settings.extensions.find(ext) == m_settings.extensions.end())
            continue;

        data::source::SourceFileEntry file;
        file.absolutePath = absPath;
        file.relativePath = fs::relative(absPath, m_settings.sourceRoot);
        file.extension    = ext;
        file.filename     = absPath.filename().string();

        outIndex.add(std::move(file));
    }
}
