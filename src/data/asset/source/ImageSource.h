#pragma once

#include "data/asset/source/AssetSourceBase.h"
#include "BinaryData.h"

#include <cstdint>
#include <vector>

namespace resource
{
    enum class ImageFormat
    {
        Unknown,
        DDS,
        TGA,
        PNG,
        JPG,
        BMP
    };

    struct ImageSource : public AssetSourceBase
    {
        BinaryData bytes;// raw file bytes

        ImageFormat format = ImageFormat::Unknown;

        uint32_t width    = 0;
        uint32_t height   = 0;
        uint32_t channels = 0;

        // Optional, only if loader decodes to raw pixels
        std::vector<uint8_t> pixelData;

        bool hasPixelData() const { return !pixelData.empty(); }
    };
}
