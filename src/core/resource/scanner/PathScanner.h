#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>

namespace resource
{
    namespace fs = std::filesystem;

    // ------------------------------------------------------------
    // PathScanner
    // - scans <clientRoot>/World/** and <resourceRoot>/World/**
    // - groupKey = first subfolder under World/
    // - collects only text / hybrid-world files (whitelist)
    // - DOES NOT parse, DOES NOT interpret, DOES NOT resolve assets
    // ------------------------------------------------------------
    class PathScanner
    {
    public:
        enum class WorldSource
        {
            Client,
            Resource
        };

        struct Entry
        {
            fs::path absPath;     // absolute path on disk
            fs::path relPath;     // relative to <root>/World/ (includes group folder)
            std::string domain; // first folder under World/
            WorldSource source;   // Client or Resource
        };

        struct Group
        {
            std::string domain;
            std::vector<Entry> clientFiles;
            std::vector<Entry> resourceFiles;
        };

        struct Settings
        {
            fs::path clientRoot;
            fs::path resourceRoot;

            // Folder name under roots (usually "World")
            std::string worldFolderName = "World";

            // Text / hybrid whitelist for World model input
            std::vector<std::string> includeExt = {
                ".wld",
                ".rgn",
                ".txt",
                ".ini",
                ".cfg"
            };
        };

    public:
        std::unordered_map<std::string, Group> scan(const Settings& s) const;

    private:
        static std::string toLower(std::string s);
        static bool hasAllowedExt(const fs::path& p, const std::vector<std::string>& extsLower);

        static void scanOneRoot(
            const fs::path& root,
            const std::string& worldFolderName,
            WorldSource source,
            const std::vector<std::string>& extsLower,
            std::unordered_map<std::string, Group>& io);
    };

} // namespace resource
