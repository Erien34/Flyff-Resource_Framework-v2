#pragma once
#include <vector>
#include <cstddef>
#include <cstdint>
#include <string>

namespace asset::parser::o3d
{
struct O3DHeader;

struct PoolDirectoryResult
{
    bool found = false;

    std::size_t scanStartCursor = 0;
    std::size_t dirCursor = 0;
    std::size_t dirAfterCursor = 0;

    std::vector<std::uint32_t> relValues;
    std::vector<std::size_t>   absCursors;

    // Phase 1: best guess
    std::size_t meshPoolStart = 0;
};

class PoolDirectoryReader
{
public:
    static bool read(PoolDirectoryResult& out,
                     const std::vector<std::uint8_t>& bytes,
                     std::size_t startCursor,
                     const O3DHeader& header,
                     std::string* outError);
};
}
