#include "SourceCollector.h"

#include <unordered_map>
#include <filesystem>

namespace fs = std::filesystem;

using namespace core::source;

SourceCollector::SourceCollector(Settings s)
    : m_settings(std::move(s))
{
}

std::string SourceCollector::stemOf(const fs::path& p)
{
    // "Object3D.cpp" -> "Object3D"
    return p.stem().string();
}

bool SourceCollector::startsWith(const std::string& s, const std::string& prefix)
{
    if (prefix.size() > s.size())
        return false;
    return std::equal(prefix.begin(), prefix.end(), s.begin());
}

bool SourceCollector::containsPathToken(const fs::path& relPath, const std::string& token)
{
    // simple v1: substring match on generic_string
    // "WorldServer/xyz/Object.cpp" contains "WorldServer"
    const std::string p = relPath.generic_string();
    return (p.find(token) != std::string::npos);
}

void SourceCollector::collect(
    const data::source::SourceIndexList& index,
    const core::source::descriptor::SourceDescriptorRegistry& rules,
    std::vector<data::source::SourceGroup>& outGroups)
{
    outGroups.clear();

    if (index.empty())
        return;

    // =========================================
    // 1) GROUP BY BASENAME (Unit)
    // =========================================
    // Key: stem ("Object3D") -> group
    std::unordered_map<std::string, data::source::SourceGroup> map;

    for (const auto& f : index.files())
    {
        // v1: group key = stem of filename
        const std::string key = stemOf(f.absolutePath);

        auto& g = map[key];
        if (g.id.empty())
        {
            g.id = key;
            g.baseDirectory = f.absolutePath.parent_path();
        }

        // baseDirectory best-effort: keep first
        g.files.push_back(f);
    }

    // =========================================
    // 2) FINALIZE + APPLY RULES
    // =========================================
    outGroups.reserve(map.size());

    for (auto& [_, g] : map)
    {
        if (m_settings.applyDescriptorRules)
            applyRulesToGroup(g, rules);

        outGroups.emplace_back(std::move(g));
    }
}

void SourceCollector::applyRulesToGroup(
    data::source::SourceGroup& group,
    const core::source::descriptor::SourceDescriptorRegistry& rules)
{
    using core::source::descriptor::SourceRuleType;

    // Helper: pick a representative file/relative path for dir-contains checks
    // If multiple files exist, any match triggers rule.
    auto anyFileMatchesPrefix = [&](const std::string& prefix) -> bool
    {
        // prefix match on group.id OR on filenames
        if (startsWith(group.id, prefix))
            return true;

        for (const auto& f : group.files)
        {
            if (startsWith(f.filename, prefix))
                return true;
        }
        return false;
    };

    auto anyFileMatchesDirContains = [&](const std::string& token) -> bool
    {
        for (const auto& f : group.files)
        {
            if (containsPathToken(f.relativePath, token))
                return true;
        }
        return false;
    };

    auto applyCategory = [&](const std::string& cat)
    {
        if (cat.empty())
            return;

        if (group.category.empty())
        {
            group.category = cat;
            return;
        }

        if (!m_settings.keepFirstCategory)
            group.category = cat;
    };

    // Iterate over all descriptors and their rules
    for (const auto& desc : rules.descriptors())
    {
        bool matchedThisDescriptor = false;

        for (const auto& rule : desc.rules)
        {
            switch (rule.type)
            {
            case SourceRuleType::PairBasename:
                // Basename grouping ist bereits umgesetzt.
                // v1: rule kann ignoriert werden, existiert nur als "policy marker".
                // (Später könntest du hier z.B. allowed extensions durchsetzen.)
                matchedThisDescriptor = true;
                break;

            case SourceRuleType::Prefix:
                if (!rule.value.empty() && anyFileMatchesPrefix(rule.value))
                {
                    matchedThisDescriptor = true;
                    applyCategory(rule.category);
                }
                break;

            case SourceRuleType::DirectoryContains:
                if (!rule.value.empty() && anyFileMatchesDirContains(rule.value))
                {
                    matchedThisDescriptor = true;
                    applyCategory(rule.category);
                }
                break;
            }
        }

        if (matchedThisDescriptor)
            group.matchedDescriptorIds.push_back(desc.id);
    }
}
