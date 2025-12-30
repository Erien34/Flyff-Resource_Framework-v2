#include "ModelNormalizer.h"

namespace asset::normalizer
{
    bool ModelNormalizer::normalizeMesh(
        asset::normalized::NormalizedMesh& out,
        const resource::O3DParsed& src,
        std::string* outError
    )
    {
        out = {};

        // 1️⃣ Validierung
        if (!src.mesh.exists)
        {
            if (outError)
                *outError = "ModelNormalizer: no mesh part present in O3DParsed.";
            return false;
        }

        if (src.mesh.raw.empty())
        {
            if (outError)
                *outError = "ModelNormalizer: mesh raw data empty.";
            return false;
        }

        // 2️⃣ PLACEHOLDER-GEOMETRIE
        // --------------------------------
        // Damit GLB-Export JETZT funktioniert
        // ersetzen wir später durch echten O3D-Parser

        out.positions = {
            { 0.0f,  0.5f, 0.0f },
            { -0.5f, -0.5f, 0.0f },
            { 0.5f, -0.5f, 0.0f }
        };

        out.normals = {
            { 0,0,1 },
            { 0,0,1 },
            { 0,0,1 }
        };

        out.uvs0 = {
            { 0.5f, 1.0f },
            { 0.0f, 0.0f },
            { 1.0f, 0.0f }
        };

        out.indices = { 0,1,2 };

        asset::normalized::NormalizedPrimitive prim;
        prim.indexOffset  = 0;
        prim.indexCount   = 3;
        prim.materialSlot = 0;

        out.primitives.push_back(prim);

        return true;
    }
}
