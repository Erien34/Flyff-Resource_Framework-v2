#pragma once

#include <vector>
#include <string>

#include "data/source/index/SourceIndexList.h"
#include "data/source/collect/SourceGroup.h"

#include "core/source/descriptor/SourceDescriptorRegistry.h"
#include "core/source/descriptor/SourceDescriptor.h"

namespace core::source
{

class SourceCollector
{
public:
    struct Settings
    {
        // v1: Basename-Gruppierung immer aktiv
        bool groupByBasename = true;

        // v1: Category/Tags anwenden
        bool applyDescriptorRules = true;

        // Wenn mehrere Regeln matchen:
        // - true: erste Category gewinnt
        // - false: überschreibt Category bei späterem Match
        bool keepFirstCategory = true;
    };

public:
    explicit SourceCollector(Settings s = {});

    void collect(
        const data::source::SourceIndexList& index,
        const core::source::descriptor::SourceDescriptorRegistry& rules,
        std::vector<data::source::SourceGroup>& outGroups
    );

private:
    Settings m_settings;

private:
    static std::string stemOf(const std::filesystem::path& p);
    static bool containsPathToken(const std::filesystem::path& relPath, const std::string& token);
    static bool startsWith(const std::string& s, const std::string& prefix);

    void applyRulesToGroup(
        data::source::SourceGroup& group,
        const core::source::descriptor::SourceDescriptorRegistry& rules
    );
};

} // namespace core::source
