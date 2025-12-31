#pragma once
#include <cstdint>

namespace core::source::assemble::model
{
    struct SubMeshSpec
    {
        uint32_t startIndex = 0;
        uint32_t primitiveCount = 0;
        uint32_t materialSlot = 0;
    };
}
