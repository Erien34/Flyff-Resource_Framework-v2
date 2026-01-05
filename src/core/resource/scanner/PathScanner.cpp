#include "PathScanner.h"

#include <algorithm>
#include <cctype>

namespace resource
{
static std::string normalizeLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

std::string PathScanner::toLower(std::string s)
{
    return normalizeLower(std::move(s));
}

bool PathScanner::hasAllowedExt(const fs::path& p, const std::vector<std::string>& extsLower)
{
    const std::string ext = toLower(p.extension().string());
    if (ext.empty())
        return false;

    for (const auto& e : extsLower)
    {
        if (ext == e)
            return true;
    }
    return false;
}

void PathScanner::scanOneRoot(
    const fs::path& root,
    const std::string& worldFolderName,
    WorldSource source,
    const std::vector<std::string>& extsLower,
    std::unordered_map<std::string, Group>& io)
{
    if (root.empty() || !fs::exists(root))
        return;

    const fs::path worldRoot = root / worldFolderName;
    if (!fs::exists(worldRoot) || !fs::is_directory(worldRoot))
        return;

    for (const auto& it : fs::recursive_directory_iterator(worldRoot))
    {
        if (!it.is_regular_file())
            continue;

        const fs::path abs = it.path();

        // only text / hybrid world files
        if (!hasAllowedExt(abs, extsLower))
            continue;

        fs::path relToWorld;
        try { relToWorld = fs::relative(abs, worldRoot); }
        catch (...) { continue; }

        if (relToWorld.empty())
            continue;

        // groupKey = first folder under World/
        auto itBeg = relToWorld.begin();
        if (itBeg == relToWorld.end())
            continue;

        const std::string domain = itBeg->string();
        if (domain.empty())
            continue;

        Entry e;
        e.absPath  = abs;
        e.relPath  = relToWorld;
        e.domain = domain;
        e.source   = source;

        const std::string mapKey = toLower(domain);

        auto& g = io[mapKey];
        if (g.domain.empty())
            g.domain = domain;

        if (source == WorldSource::Client)
            g.clientFiles.push_back(std::move(e));
        else
            g.resourceFiles.push_back(std::move(e));
    }
}

std::unordered_map<std::string, PathScanner::Group> PathScanner::scan(const Settings& s) const
{
    std::unordered_map<std::string, Group> out;

    std::vector<std::string> extsLower;
    extsLower.reserve(s.includeExt.size());
    for (const auto& e : s.includeExt)
        extsLower.push_back(toLower(e));

    scanOneRoot(s.clientRoot,   s.worldFolderName, WorldSource::Client,   extsLower, out);
    scanOneRoot(s.resourceRoot, s.worldFolderName, WorldSource::Resource, extsLower, out);

    // deterministic order
    for (auto& kv : out)
    {
        auto& g = kv.second;

        auto sortByRel = [](const Entry& a, const Entry& b)
        {
            return PathScanner::toLower(a.relPath.generic_string()) <
                   PathScanner::toLower(b.relPath.generic_string());
        };

        std::sort(g.clientFiles.begin(), g.clientFiles.end(), sortByRel);
        std::sort(g.resourceFiles.begin(), g.resourceFiles.end(), sortByRel);
    }

    return out;
}

} // namespace resource
