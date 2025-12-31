#include "ImageDecoder.h"
#include "TgaDecoder.h"
#include <cstdint>
#include "stb_image.h"

namespace asset
{
    static bool isDDS(const BinaryData& bytes)
    {
        if (bytes.size() < 4) return false;
        const std::uint8_t* p = bytes.data();
        return p[0] == 'D' && p[1] == 'D' && p[2] == 'S' && p[3] == ' ';
    }

    static bool isTGA(const BinaryData& bytes)
    {
        if (bytes.size() < 18) return false;
        const std::uint8_t imageType = bytes.data()[2];
        // 2 = uncompressed true-color, 10 = RLE true-color, 3/11 = grayscale
        return imageType == 2 || imageType == 10 || imageType == 3 || imageType == 11;
    }

    bool ImageDecoder::shouldDecode(const BinaryData& bytes)
    {
        return isDDS(bytes) || isTGA(bytes);
    }

    bool decodeWithStb(
        DecodedImageData& out,
        const BinaryData& bytes,
        std::string* err)
    {
        int w = 0, h = 0, comp = 0;

        // 🔒 Immer RGBA erzwingen
        unsigned char* data = stbi_load_from_memory(
            bytes.data(),
            (int)bytes.size(),
            &w, &h, &comp,
            4 // force RGBA
            );

        if (!data)
        {
            if (err) *err = stbi_failure_reason();
            return false;
        }

        out.width  = w;
        out.height = h;
        out.mipCount = 1;
        out.premultipliedAlpha = false;

        out.rgba.assign(data, data + (w * h * 4));
        stbi_image_free(data);

        return true;
    }

    bool ImageDecoder::decode(
        DecodedImageData& out,
        const BinaryData& bytes,
        std::string* err)
    {
        if (isDDS(bytes))
            return DdsDecoder::decode(out, bytes, err);

        if (isTGA(bytes))
            return TgaDecoder::decode(out, bytes, err);

        // 🔥 Alles andere: BMP / PNG / JPG / GIF etc.
        return decodeWithStb(out, bytes, err);
    }
}
