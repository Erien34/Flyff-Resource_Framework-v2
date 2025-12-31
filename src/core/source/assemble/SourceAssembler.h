#pragma once
#include <vector>

#include "data/source/extract/ExtractFact.h"
#include "data/source/spec/MeshRenderSpec.h"

namespace core::source::assemble
{
    class SourceAssembler
    {
    public:
        void assemble(
            const std::vector<core::source::extract::ExtractFact>& facts,
            model::MeshRenderSpec& outSpec
        ) const;
    };
}
