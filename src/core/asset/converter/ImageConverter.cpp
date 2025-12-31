#include "ImageConverter.h"
#include "core/asset/decoder/ImageDecoder.h"
#include <stb_image_write.h>

namespace
{
bool writePngRGBA(
    const std::filesystem::path& outPath,
    int width,
    int height,
    const uint8_t* rgba,
    std::string* error)
{
    if (!rgba || width <= 0 || height <= 0)
    {
        if (error) *error = "writePngRGBA: invalid image data";
        return false;
    }

    const int stride = width * 4;

    if (!stbi_write_png(
            outPath.string().c_str(),
            width,
            height,
            4,
            rgba,
            stride))
    {
        if (error)
            *error = "stbi_write_png failed: " + outPath.string();
        return false;
    }

    return true;
}
} // anonymous namespace

namespace asset
{
ConvertResult ImageConverter::convert(const ImageSource& src) const
{
    ConvertResult r;

    // 🔒 Quelle & Ziel kommen aus AssetSourceBase
    auto outPng = src.outPath;   // Kopie (nicht const)
    outPng.replace_extension(".png");

    if (shouldSkipWrite(outPng))
    {
        r.ok = true;
        return r;
    }

    DecodedImageData dec;
    std::string err;

    if (!ImageDecoder::decode(dec, src.bytes, &err) || dec.rgba.empty())
    {
        r.ok = false;
        r.error = "Image decode failed: " + err;
        return r;
    }

    if (!ensureParentDir(outPng, &err))
    {
        r.ok = false;
        r.error = err;
        return r;
    }

    if (!writePngRGBA(outPng, dec.width, dec.height, dec.rgba.data(), &err))
    {
        r.ok = false;
        r.error = err;
        return r;
    }

    r.ok = true;
    return r;
}
} // namespace asset
