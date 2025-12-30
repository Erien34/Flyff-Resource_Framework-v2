#pragma once
#include <vector>
#include <cstdint>

struct NormalizedPrimitive
{
    // geometry
    std::vector<float> positions;   // xyz
    std::vector<float> normals;     // xyz
    std::vector<float> uvs;         // uv
    std::vector<uint32_t> indices;

    // material binding (index into NormalizedMesh.materials)
    int materialIndex = -1;
};