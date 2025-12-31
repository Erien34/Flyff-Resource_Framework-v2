#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace core::source::extract::util
{
    struct TextUnit
    {
        std::filesystem::path absPath;
        std::filesystem::path relPath;

        std::string content;
        std::vector<int> lineOffsets; // 0-based offsets, line 1 starts at offsets[0]=0

        static TextUnit load(const std::filesystem::path& abs, const std::filesystem::path& rel);

        int lineOfOffset(std::size_t off) const;              // 1-based
        std::string snippetAtLine(int line1Based, int ctx=2) const;
    };

    std::string jsonEscape(std::string_view s);
}
