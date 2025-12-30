#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>

namespace asset::parser::o3d
{
struct VertexBufferResult
{
    bool found = false;

    std::size_t vertexDataStart = 0;
    std::size_t vertexStride    = 0;
    std::size_t vertexCount     = 0;

    std::vector<std::uint8_t> raw; // vertexCount * vertexStride
};

class VertexBufferReader
{
public:
    static bool read(VertexBufferResult& out,
                     const std::vector<std::uint8_t>& bytes,
                     std::size_t startCursor,
                     std::string* outError);
};
}
