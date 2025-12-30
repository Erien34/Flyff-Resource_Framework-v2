#pragma once

#include <filesystem>
#include <unordered_set>

#include "data/source/index/SourceIndexList.h"

namespace core::source
{

class SourceScanner
{
public:
    struct Settings
    {
        std::filesystem::path sourceRoot;

        std::unordered_set<std::string> extensions = {
            ".cpp", ".h", ".hpp", ".inl"
        };
    };

    explicit SourceScanner(Settings settings);

    void scan(data::source::SourceIndexList& outIndex);

private:
    Settings m_settings;
};

} // namespace core::source
