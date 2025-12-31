#pragma once
#include <string>

namespace core::source::extract
{
    struct ExtractFact
    {
        // z.B. "SubMesh.KeywordHit", "DrawCall.Symbol", "Renderer.LoopPattern"
        std::string factType;

        // stabiler Ursprung
        std::string extractorId;   // z.B. "SubMeshDrawCall"
        int extractorVersion = 1;

        // Provenance
        std::string file;          // relative oder absolute, je nach Wunsch
        int line = -1;             // 1-based
        float confidence = 1.0f;

        // v1: Payload als JSON-ish string (später kann das typed werden)
        std::string payload;
    };
}
