#pragma once
#include <vector>
#include <string>

#include "data/source/collect/SourceGroup.h"
#include "data/source/extract/ExtractFact.h"

namespace core::source::extract
{
    class RuleExtractorBase
    {
    public:
        virtual ~RuleExtractorBase() = default;

        virtual std::string id() const = 0;
        virtual int version() const = 0;

        virtual void extract(
            const std::vector<data::source::SourceGroup>& groups,
            std::vector<ExtractFact>& outFacts
        ) const = 0;
    };
}
