#pragma once
#include "data/asset/decoded/DecodedImageData.h"
#include "index/AssetIndexBuilder.h"
#include "data/asset/source/BinaryData.h"
#include <string>

namespace asset
{
    // Unified decoder for FlyFF-specific / non-standard formats.
    // Only DDS and TGA require custom decoding; standard formats should be handled elsewhere.
    class ImageDecoder
    {
    public:
        static bool decode(DecodedImageData& out, const BinaryData& bytes, std::string* err);
        static bool shouldDecode(const BinaryData& bytes);
        static bool isSupportedByStb(const BinaryData& bytes);
    };
}
