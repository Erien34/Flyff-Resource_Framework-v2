#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>

namespace asset::parser::o3d
{
struct PoolDirectoryResult;

struct MeshEntry
{
    std::size_t cursorAtMeshStart = 0;
    std::size_t cursorAfterMesh = 0;

    int meshId = 0;
    int vertexCount = 0;
    int indexCount = 0;
};

struct MeshReadResult
{
    int meshCount = 0;
    std::vector<MeshEntry> meshes;
    std::size_t cursorAfterMeshes = 0;
};

class MeshReader
{
public:
    static bool read(MeshReadResult& out,
                     const std::vector<std::uint8_t>& bytes,
                     std::size_t startCursor,
                     const PoolDirectoryResult* dirOpt,
                     std::string* outError);
};
} // namespace asset::parser::o3d
