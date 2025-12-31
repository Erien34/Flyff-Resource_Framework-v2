#pragma once
#include <memory>
#include <vector>

#include "data/source/collect/SourceGroup.h"
#include "data/source/extract/ExtractFact.h"
#include "core/source/extract/schema/RuleExtractorBase.h"

namespace core::source::extract
{
    class SourceExtractor
    {
    public:
        struct Settings
        {
            // v1: feste Reihenfolge, deterministisch
            bool enableSubMeshSchema = true;
            bool enableDrawCallSchema = true;
        };

        explicit SourceExtractor(Settings s = Settings{});

        // EINZIGE Pipeline-Schnittstelle
        void extract(
            const std::vector<data::source::SourceGroup>& groups,
            std::vector<ExtractFact>& outFacts
        ) const;

        std::size_t extractorCount() const { return m_extractors.size(); }

    private:
        Settings m_settings;

        // SourceExtractor OWNS lifetime
        std::vector<std::unique_ptr<RuleExtractorBase>> m_extractors;

        void buildDefaultExtractors(); // erstellt die Extractoren genau 1x
    };
}
