#include "core/source/assemble/SourceAssembler.h"

using core::source::extract::ExtractFact;
using namespace core::source::assemble::model;

namespace core::source::assemble
{
void SourceAssembler::assemble(
    const std::vector<core::source::extract::ExtractFact>& facts,
    model::MeshRenderSpec& outSpec
    ) const
{
    using namespace core::source::assemble::model;

    outSpec = {};

    bool hasStruct = false;
    bool hasArrayRead = false;

    for (const auto& f : facts)
    {
        // --- SubMesh schema evidence ---
        if (f.factType == "SubMesh.Block.Struct")
            hasStruct = true;
        else if (f.factType == "SubMesh.Block.ArrayRead")
            hasArrayRead = true;

        // --- Drawcall schema ---
        else if (f.factType == "DrawCall.PrimitiveType.TriangleList")
            outSpec.draw.primitiveType = PrimitiveType::TriangleList;

        else if (f.factType == "DrawCall.Invoke.DrawIndexedPrimitive")
            outSpec.draw.usesDrawIndexedPrimitive = true; // <-- add field (see below)

        else if (f.factType == "DrawCall.PerSubMesh")
            outSpec.draw.perSubMesh = true;

        else if (f.factType == "DrawCall.Grouping.SetGroup")
            outSpec.draw.usesLOD = true;

        else if (f.factType == "Material.Variant.SetTextureEx")
            outSpec.draw.usesMaterialVariants = true;

        else if (f.factType == "DrawCall.Mapping.MaterialBlockField")
            outSpec.draw.mappingFieldLocations.push_back(
                f.file + ":" + std::to_string(f.line)
                );
    }

    outSpec.draw.hasMaterialBlockEvidence = (hasStruct && hasArrayRead); // <-- add field
}
}
