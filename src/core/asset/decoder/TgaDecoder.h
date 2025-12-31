#pragma once
#include "data/asset/decoded/DecodedImageData.h"
#include "data/asset/source/BinaryData.h"
#include <string>

namespace asset
{
    // Minimal TGA decoder:
    // - Supports true-color TGA (uncompressed: type 2, RLE: type 10) with 24/32 bpp
    // - Supports grayscale TGA (uncompressed: type 3, RLE: type 11) with 8 bpp
    // - Rejects color-mapped TGAs (colorMapType != 0)
    class TgaDecoder
    {
    public:
        static bool decode(DecodedImageData& out, const BinaryData& bytes, std::string* err);
    };
}
