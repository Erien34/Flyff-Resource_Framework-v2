#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include "asset/parser/o3d/HeaderReader.h"

namespace asset::parser::o3d
{
struct O3DObjectInfo
{
    std::int32_t unk0 = 0;                 // häufig -1
    float matrix[16] = {};                 // D3DXMATRIX (row-major)

    std::size_t cursorAtObjectStart = 0;
    std::size_t cursorAfterMatrix   = 0;
    std::int32_t unkA = 0;
    std::int32_t unkB = 0;
    float params[6] = {};      // entspricht w2–w7
    std::uint32_t poolIndex = 0;
    std::size_t cursorAfterParams = 0;
    bool hasMesh =true;
    bool hasMeshCandidate = false;
};

struct O3DGroupInfo
{
    std::int32_t objectCount = 0;
    std::vector<O3DObjectInfo> objects;
};

struct ObjectReadResult
{
    std::int32_t groupCount = 0;
    std::vector<O3DGroupInfo> groups;
    std::size_t cursorAfterObjects = 0;
};

class ObjectReader
{
public:
    static bool read(ObjectReadResult& out,
                     const std::vector<std::uint8_t>& bytes,
                     const O3DHeader& header,
                     std::string* outError = nullptr);
};
}
