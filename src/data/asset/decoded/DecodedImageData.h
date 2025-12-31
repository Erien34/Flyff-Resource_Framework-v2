#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace asset
{
    struct BinaryData;

    struct DecodedImageData
    {
        int width = 0;
        int height = 0;
        int mipCount = 1;

        // DXT2/DXT4 sind premultiplied alpha Varianten
        bool premultipliedAlpha = false;

        // RGBA8 (width*height*4)
        std::vector<std::uint8_t> rgba;
    };

    class DdsDecoder
    {
    public:
        // Memory-only: erwartet vollständige DDS-Dateibytes
        static bool decode(DecodedImageData& out, const BinaryData& inBytes, std::string* outError = nullptr);
    };
}