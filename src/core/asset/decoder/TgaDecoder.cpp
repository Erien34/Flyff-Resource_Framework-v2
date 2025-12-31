#include "TgaDecoder.h"

#include <cstdint>
#include <cstring>

namespace asset
{
#pragma pack(push, 1)
    struct TgaHeader
    {
        std::uint8_t  idLength;
        std::uint8_t  colorMapType;
        std::uint8_t  imageType;
        std::uint16_t colorMapFirstEntryIndex;
        std::uint16_t colorMapLength;
        std::uint8_t  colorMapEntrySize;
        std::uint16_t xOrigin;
        std::uint16_t yOrigin;
        std::uint16_t width;
        std::uint16_t height;
        std::uint8_t  pixelDepth;
        std::uint8_t  imageDescriptor;
    };
#pragma pack(pop)

    static bool fail(std::string* err, const char* msg)
    {
        if (err) *err = msg;
        return false;
    }

    static bool safeRead(const BinaryData& b, std::size_t off, void* dst, std::size_t n)
    {
        if (off + n > b.size()) return false;
        std::memcpy(dst, b.data() + off, n);
        return true;
    }

    static void writePixelRGBA(std::uint8_t* dst, std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a)
    {
        dst[0] = r; dst[1] = g; dst[2] = b; dst[3] = a;
    }

    bool TgaDecoder::decode(DecodedImageData& out, const BinaryData& bytes, std::string* err)
    {
        out = {};

        if (bytes.size() < sizeof(TgaHeader))
            return fail(err, "TGA: file too small");

        TgaHeader h{};
        if (!safeRead(bytes, 0, &h, sizeof(TgaHeader)))
            return fail(err, "TGA: cannot read header");

        if (h.colorMapType != 0)
            return fail(err, "TGA: color-mapped TGAs are not supported");

        const bool rle = (h.imageType == 10) || (h.imageType == 11);
        const bool trueColor = (h.imageType == 2) || (h.imageType == 10);
        const bool gray = (h.imageType == 3) || (h.imageType == 11);

        if (!trueColor && !gray)
            return fail(err, "TGA: unsupported imageType (expected 2/10 true-color or 3/11 grayscale)");

        const std::uint32_t w = h.width;
        const std::uint32_t hgt = h.height;

        if (w == 0 || hgt == 0)
            return fail(err, "TGA: invalid dimensions");

        const std::uint32_t bpp = h.pixelDepth;
        if (trueColor && !(bpp == 24 || bpp == 32))
            return fail(err, "TGA: true-color must be 24 or 32 bpp");

        if (gray && bpp != 8)
            return fail(err, "TGA: grayscale must be 8 bpp");

        const std::uint32_t srcPixelSize = bpp / 8;
        std::size_t offset = sizeof(TgaHeader) + static_cast<std::size_t>(h.idLength);
        if (offset > bytes.size())
            return fail(err, "TGA: invalid ID length");

        const std::uint64_t pixelCount64 = static_cast<std::uint64_t>(w) * static_cast<std::uint64_t>(hgt);
        if (pixelCount64 > (1ull << 31))
            return fail(err, "TGA: image too large");

        const std::size_t pixelCount = static_cast<std::size_t>(pixelCount64);

        out.width = static_cast<int>(w);
        out.height = static_cast<int>(hgt);
        out.rgba.resize(pixelCount * 4);

        std::uint8_t* dst = out.rgba.data();
        const bool originTop = (h.imageDescriptor & 0x20) != 0;

        auto putPixelAt = [&](std::size_t linearIndex, std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a)
        {
            std::size_t x = linearIndex % w;
            std::size_t y = linearIndex / w;
            if (!originTop)
                y = (hgt - 1) - y;

            std::size_t outIndex = (y * w + x) * 4;
            writePixelRGBA(dst + outIndex, r, g, b, a);
        };

        auto readOnePixel = [&](std::size_t& off, std::uint8_t& r, std::uint8_t& g, std::uint8_t& b, std::uint8_t& a) -> bool
        {
            if (off + srcPixelSize > bytes.size())
                return false;

            const std::uint8_t* p = bytes.data() + off;
            off += srcPixelSize;

            if (gray)
            {
                r = g = b = p[0];
                a = 255;
                return true;
            }

            b = p[0];
            g = p[1];
            r = p[2];
            a = (srcPixelSize == 4) ? p[3] : 255;
            return true;
        };

        if (!rle)
        {
            for (std::size_t i = 0; i < pixelCount; ++i)
            {
                std::uint8_t r = 0, g = 0, b = 0, a = 255;
                if (!readOnePixel(offset, r, g, b, a))
                    return fail(err, "TGA: unexpected end of file while reading pixels");

                putPixelAt(i, r, g, b, a);
            }
            return true;
        }

        std::size_t written = 0;
        while (written < pixelCount)
        {
            if (offset >= bytes.size())
                return fail(err, "TGA: unexpected end of file in RLE stream");

            std::uint8_t packetHeader = bytes.data()[offset++];
            std::size_t count = (packetHeader & 0x7Fu) + 1u;

            if (packetHeader & 0x80u)
            {
                std::uint8_t r = 0, g = 0, b = 0, a = 255;
                if (!readOnePixel(offset, r, g, b, a))
                    return fail(err, "TGA: unexpected end of file in RLE pixel");

                for (std::size_t k = 0; k < count && written < pixelCount; ++k)
                    putPixelAt(written++, r, g, b, a);
            }
            else
            {
                for (std::size_t k = 0; k < count && written < pixelCount; ++k)
                {
                    std::uint8_t r = 0, g = 0, b = 0, a = 255;
                    if (!readOnePixel(offset, r, g, b, a))
                        return fail(err, "TGA: unexpected end of file in raw packet");

                    putPixelAt(written++, r, g, b, a);
                }
            }
        }

        return true;
    }
}
