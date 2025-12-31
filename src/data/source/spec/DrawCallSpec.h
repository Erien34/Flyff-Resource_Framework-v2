#pragma once
#include <string>
#include <vector>
#include "SubMeshSpec.h"

namespace core::source::assemble::model
{
    enum class PrimitiveType
    {
        Unknown,
        TriangleList
    };

    struct DrawCallSpec
    {
        PrimitiveType primitiveType = PrimitiveType::Unknown;

        bool usesDrawIndexedPrimitive = false;   // NEW
        bool perSubMesh = false;
        bool usesLOD = false;
        bool usesMaterialVariants = false;

        bool hasMaterialBlockEvidence = false;   // NEW (struct + array read)

        std::vector<std::string> mappingFieldLocations;

        std::vector<SubMeshSpec> subMeshes; // bleibt leer in v1 (weil wir hier nur Schema assemblen)
    };
}
