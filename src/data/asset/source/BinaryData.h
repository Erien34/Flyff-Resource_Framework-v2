#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

namespace resource
{
    // Simple owned byte buffer used by loaders/decoders.
    struct BinaryData
    {
    std::vector<std::uint8_t> bytes;

    void clear() { bytes.clear(); }
    bool empty() const { return bytes.empty(); }
    std::size_t size() const { return bytes.size(); }

    std::uint8_t* data() { return bytes.empty() ? nullptr : bytes.data(); }
    const std::uint8_t* data() const { return bytes.empty() ? nullptr : bytes.data(); }
    };
}
