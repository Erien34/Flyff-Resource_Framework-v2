#include "core/source/extract/SourceExtract.h"

// include concrete extractors ONLY here (Pipeline bleibt clean)
#include "core/source/extract/schema/SubMeshSchemaExtractor.h"
#include "core/source/extract/schema/DrawCallSchemaExtractor.h"

namespace core::source::extract
{
    SourceExtractor::SourceExtractor(Settings s)
        : m_settings(std::move(s))
    {
        buildDefaultExtractors();
    }

    void SourceExtractor::buildDefaultExtractors()
    {
        using namespace core::source::extract::rules;

        m_extractors.clear();
        m_extractors.reserve(4);

        if (m_settings.enableSubMeshSchema)
            m_extractors.emplace_back(std::make_unique<SubMeshSchemaExtractor>());

        if (m_settings.enableDrawCallSchema)
            m_extractors.emplace_back(std::make_unique<DrawCallSchemaExtractor>());
    }

    void SourceExtractor::extract(
        const std::vector<data::source::SourceGroup>& groups,
        std::vector<ExtractFact>& outFacts
    ) const
    {
        for (const auto& ex : m_extractors)
            ex->extract(groups, outFacts);
    }
}
