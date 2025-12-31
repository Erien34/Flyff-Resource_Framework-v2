#pragma once
#include "core/source/extract/schema/RuleExtractorBase.h"

namespace core::source::extract::rules
{
    class SubMeshSchemaExtractor final : public RuleExtractorBase
    {
    public:
        std::string id() const override { return "SubMeshSchema"; }
        int version() const override { return 1; }

        void extract(const std::vector<data::source::SourceGroup>& groups,
                     std::vector<ExtractFact>& outFacts) const override;
    };
}
