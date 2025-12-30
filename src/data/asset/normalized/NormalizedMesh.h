#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <array>

namespace asset::normalized
{
    struct Vec2 { float x=0, y=0; };
    struct Vec3 { float x=0, y=0, z=0; };

    // Ein Primitive = eine Drawcall-Gruppe (meist Material-gebunden)
    struct NormalizedPrimitive
    {
        uint32_t indexOffset = 0;   // Start in indices[]
        uint32_t indexCount  = 0;   // Anzahl Indices (multiple of 3)
        int32_t  materialSlot = -1; // referenziert NormalizedMaterial-Index (später)
        std::string debugName;
    };

    struct NormalizedMesh
    {
        std::string name;

        // Attribute-Arrays (interleaved später im Writer möglich)
        std::vector<Vec3> positions;
        std::vector<Vec3> normals;
        std::vector<Vec2> uvs0;

        // Triangle index buffer (uint32 ok für GLTF)
        std::vector<uint32_t> indices;

        // Subsets / primitives (Materialgruppen)
        std::vector<NormalizedPrimitive> primitives;

        bool hasNormals() const { return normals.size() == positions.size(); }
        bool hasUvs0()    const { return uvs0.size()    == positions.size(); }
    };
}
