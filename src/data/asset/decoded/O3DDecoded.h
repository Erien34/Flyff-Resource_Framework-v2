#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace resource
{
    struct O3DDecodedPart
    {
        bool exists = false;
        std::string extension;          // z.B. ".o3d" / ".ase" (lower-case, aus Loader)
        std::uint32_t sizeBytes = 0;
        std::string signatureHex;       // erste 32 bytes als hex (debug/diagnose)
        std::vector<std::uint8_t> raw;  // unverändert (memory-only)
    };

    struct O3DDecoded
    {
        // entspricht deinem Loader-Design:
        // skeleton = client-part, mesh = resource-part
        O3DDecodedPart skeleton;
        O3DDecodedPart mesh;

        bool hasSkeleton() const { return skeleton.exists; }
        bool hasMesh() const { return mesh.exists; }
    };
}