#pragma once
#include "core/asset/converter/AssetConverterBase.h"
#include "data/asset/source/ImageSource.h"
#include "data/asset/decoded/DecodedImageData.h"

namespace resource
{
// Adapter: hier dockst du an deine Custom STB-PNG-Write Funktion an.
struct PngEncoder
{
    // Muss RGBA8 annehmen (w*h*4).
    static bool writeRGBA(const std::filesystem::path& outPng,
                          int w, int h,
                          const std::vector<uint8_t>& rgba,
                          std::string* err);
};

class ImageConverter : public AssetConverterBase
{
public:
    using AssetConverterBase::AssetConverterBase;

    ConvertResult convert(const ImageSource& src) const;
};
} // namespace resource
