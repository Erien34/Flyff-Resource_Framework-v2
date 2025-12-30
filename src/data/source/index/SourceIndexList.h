#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace data::source
{

    struct SourceFileEntry
    {
        std::filesystem::path absolutePath;
        std::filesystem::path relativePath;

        std::string extension;
        std::string filename;
    };

    class SourceIndexList
    {
    public:
        void clear() { m_files.clear(); }

        void add(SourceFileEntry entry)
        {
            m_files.emplace_back(std::move(entry));
        }

        const std::vector<SourceFileEntry>& files() const
        {
            return m_files;
        }

        bool empty() const { return m_files.empty(); }
        std::size_t size() const { return m_files.size(); }

    private:
        std::vector<SourceFileEntry> m_files;
    };

} // namespace data::source
