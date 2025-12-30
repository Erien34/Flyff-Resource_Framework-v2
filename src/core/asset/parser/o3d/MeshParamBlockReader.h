#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace asset::parser::o3d
{
struct MeshParamBlock
{
    uint32_t vertexCount   = 0;
    uint32_t indexCount    = 0;
    uint32_t vertexStride  = 0;
    uint32_t vertexOffset  = 0; // rel to meshPoolStart
    uint32_t indexOffset   = 0; // rel to meshPoolStart

    uint32_t primitiveType = 0;
    uint32_t vertexFormat  = 0;
    uint32_t materialIndex = 0;
    uint32_t unkExtra      = 0;
};

struct SubMesh
{
    uint32_t indexStart;
    uint32_t indexCount;
    uint32_t materialIndex;
    uint32_t primitiveType;
};

struct MeshParamBlockResult
{
    bool found = false;

    bool isRenderable;
    std::size_t paramBlockStartAbs = 0;
    std::size_t cursorAfter        = 0;

    MeshParamBlock block;

    std::vector<SubMesh> subMeshes;
};

class MeshParamBlockReader
{
public:
    // Der Reader entscheidet intern, welcher Cursor korrekt ist.
    static bool read(
        MeshParamBlockResult& out,
        const std::vector<uint8_t>& raw,
        std::size_t cursorAfterMatrix,
        std::size_t cursorAfterParams,
        std::size_t meshPoolStart,
        uint32_t version,
        std::string* err);
};

} // namespace asset::parser::o3d
