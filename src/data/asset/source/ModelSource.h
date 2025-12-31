#pragma once

#include "data/asset/source/AssetSourceBase.h"
#include "BinaryData.h"

#include <string>

namespace asset
{
    // FlyFF models are typically split across two locations:
    // - client Model/... : skeleton/bones + attachments metadata
    // - resource Model/... : mesh + collision/hitbox or additional geometry data
    // Both share the same relative path (key) and need to be merged later.

    struct ModelPartSource
    {
        std::filesystem::path path;  // absolute path
        bool exists = false;
        BinaryData bytes;            // raw file bytes (optional; filled by loader)

        // optional hints
        std::string extension;       // lower-case
    };

    struct ModelSource : public AssetSourceBase
    {
        // two-part source
        ModelPartSource skeleton; // usually client
        ModelPartSource mesh;     // usually resource

        // convenience flags
        bool hasSkeleton() const { return skeleton.exists && !skeleton.path.empty(); }
        bool hasMesh() const { return mesh.exists && !mesh.path.empty(); }
    };
}
