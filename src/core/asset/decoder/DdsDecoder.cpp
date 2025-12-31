#include "data/asset/decoded/DecodedImageData.h"
#include "asset/source/BinaryData.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>

namespace asset
{
    // ------------------------------------------------------------
    // DDS structs (packed, little-endian)
    // ------------------------------------------------------------
#pragma pack(push, 1)
    struct DDS_PIXELFORMAT
    {
        std::uint32_t size;        // must be 32
        std::uint32_t flags;
        std::uint32_t fourCC;
        std::uint32_t rgbBitCount;
        std::uint32_t rMask;
        std::uint32_t gMask;
        std::uint32_t bMask;
        std::uint32_t aMask;
    };

    struct DDS_HEADER
    {
        std::uint32_t size;        // must be 124
        std::uint32_t flags;
        std::uint32_t height;
        std::uint32_t width;
        std::uint32_t pitchOrLinearSize;
        std::uint32_t depth;
        std::uint32_t mipMapCount;
        std::uint32_t reserved1[11];
        DDS_PIXELFORMAT ddspf;
        std::uint32_t caps;
        std::uint32_t caps2;
        std::uint32_t caps3;
        std::uint32_t caps4;
        std::uint32_t reserved2;
    };

    struct DDS_HEADER_DXT10
    {
        std::uint32_t dxgiFormat;
        std::uint32_t resourceDimension;
        std::uint32_t miscFlag;
        std::uint32_t arraySize;
        std::uint32_t miscFlags2;
    };
#pragma pack(pop)

    // DDS flags / pf flags (subset)
    static constexpr std::uint32_t DDPF_ALPHAPIXELS = 0x00000001;
    static constexpr std::uint32_t DDPF_FOURCC      = 0x00000004;
    static constexpr std::uint32_t DDPF_RGB         = 0x00000040;

    // FourCC helper
    static constexpr std::uint32_t FCC(char a, char b, char c, char d)
    {
        return (std::uint32_t)(std::uint8_t)a |
               ((std::uint32_t)(std::uint8_t)b << 8) |
               ((std::uint32_t)(std::uint8_t)c << 16) |
               ((std::uint32_t)(std::uint8_t)d << 24);
    }

    // DXGI_FORMAT subset we care about (values are standard)
    // We only reference the numeric ids used by DDS DX10 header.
    // BC1: 71/72, BC2: 74/75, BC3: 77/78, BC4: 80/81, BC5: 83/84
    static bool dxgiToKind(std::uint32_t dxgi, std::uint32_t& outFourCCLike)
    {
        switch (dxgi)
        {
        case 71: // BC1_UNORM
        case 72: // BC1_UNORM_SRGB
            outFourCCLike = FCC('D','X','T','1');
            return true;
        case 74: // BC2_UNORM
        case 75: // BC2_UNORM_SRGB
            outFourCCLike = FCC('D','X','T','3');
            return true;
        case 77: // BC3_UNORM
        case 78: // BC3_UNORM_SRGB
            outFourCCLike = FCC('D','X','T','5');
            return true;
        case 80: // BC4_UNORM
        case 81: // BC4_SNORM
            outFourCCLike = FCC('A','T','I','1'); // treat as BC4
            return true;
        case 83: // BC5_UNORM
        case 84: // BC5_SNORM
            outFourCCLike = FCC('A','T','I','2'); // treat as BC5
            return true;
        default:
            return false;
        }
    }

    // ------------------------------------------------------------
    // Safe reads
    // ------------------------------------------------------------
    template <typename T>
    static bool readAt(const std::vector<std::uint8_t>& buf, std::size_t off, T& out)
    {
        if (off + sizeof(T) > buf.size())
            return false;
        std::memcpy(&out, buf.data() + off, sizeof(T));
        return true;
    }

    static bool readU32At(const std::vector<std::uint8_t>& buf, std::size_t off, std::uint32_t& out)
    {
        if (off + 4 > buf.size())
            return false;
        std::memcpy(&out, buf.data() + off, 4);
        return true;
    }

    // ------------------------------------------------------------
    // Bitmask helpers (uncompressed)
    // ------------------------------------------------------------
    static int lowestBitIndex(std::uint32_t mask)
    {
        if (mask == 0) return -1;
        int idx = 0;
        while ((mask & 1u) == 0u) { mask >>= 1u; ++idx; }
        return idx;
    }

    static int bitCount(std::uint32_t mask)
    {
        int c = 0;
        while (mask) { c += (mask & 1u) ? 1 : 0; mask >>= 1u; }
        return c;
    }

    static std::uint8_t expandBitsTo8(std::uint32_t v, int bits)
    {
        if (bits <= 0) return 0;
        if (bits >= 8) return (std::uint8_t)std::min<std::uint32_t>(v, 255u);
        // scale to 0..255
        const std::uint32_t maxv = (1u << bits) - 1u;
        return (std::uint8_t)((v * 255u + (maxv / 2u)) / maxv);
    }

    static bool decodeUncompressedToRGBA8(
        const std::vector<std::uint8_t>& src,
        std::size_t pixelDataOffset,
        int width, int height,
        const DDS_PIXELFORMAT& pf,
        std::vector<std::uint8_t>& outRGBA,
        std::string* err)
    {
        const int bpp = (int)pf.rgbBitCount;
        if (!(pf.flags & DDPF_RGB))
        {
            if (err) *err = "DDS: uncompressed but missing DDPF_RGB flag.";
            return false;
        }

        if (bpp != 16 && bpp != 24 && bpp != 32)
        {
            if (err) *err = "DDS: unsupported uncompressed rgbBitCount=" + std::to_string(bpp);
            return false;
        }

        const std::size_t bytesPerPixel = (std::size_t)bpp / 8u;
        const std::size_t needed = (std::size_t)width * (std::size_t)height * bytesPerPixel;
        if (pixelDataOffset + needed > src.size())
        {
            if (err) *err = "DDS: insufficient data for uncompressed pixels.";
            return false;
        }

        const int rShift = lowestBitIndex(pf.rMask);
        const int gShift = lowestBitIndex(pf.gMask);
        const int bShift = lowestBitIndex(pf.bMask);
        const int aShift = lowestBitIndex(pf.aMask);

        const int rBits = bitCount(pf.rMask);
        const int gBits = bitCount(pf.gMask);
        const int bBits = bitCount(pf.bMask);
        const int aBits = bitCount(pf.aMask);

        outRGBA.resize((std::size_t)width * (std::size_t)height * 4u);

        const std::uint8_t* p = src.data() + pixelDataOffset;

        auto readPixelValue = [&](std::size_t i) -> std::uint32_t
        {
            const std::uint8_t* q = p + i * bytesPerPixel;
            std::uint32_t v = 0;
            // little-endian
            if (bytesPerPixel >= 1) v |= (std::uint32_t)q[0];
            if (bytesPerPixel >= 2) v |= (std::uint32_t)q[1] << 8;
            if (bytesPerPixel >= 3) v |= (std::uint32_t)q[2] << 16;
            if (bytesPerPixel >= 4) v |= (std::uint32_t)q[3] << 24;
            return v;
        };

        for (std::size_t i = 0; i < (std::size_t)width * (std::size_t)height; ++i)
        {
            const std::uint32_t v = readPixelValue(i);

            auto extract = [&](std::uint32_t mask, int shift, int bits) -> std::uint8_t
            {
                if (mask == 0 || shift < 0 || bits <= 0) return 0;
                const std::uint32_t raw = (v & mask) >> (std::uint32_t)shift;
                return expandBitsTo8(raw, bits);
            };

            const std::uint8_t r = extract(pf.rMask, rShift, rBits);
            const std::uint8_t g = extract(pf.gMask, gShift, gBits);
            const std::uint8_t b = extract(pf.bMask, bShift, bBits);

            std::uint8_t a = 255;
            if (pf.flags & DDPF_ALPHAPIXELS)
                a = extract(pf.aMask, aShift, aBits);

            const std::size_t o = i * 4u;
            outRGBA[o + 0] = r;
            outRGBA[o + 1] = g;
            outRGBA[o + 2] = b;
            outRGBA[o + 3] = a;
        }

        return true;
    }

    // ------------------------------------------------------------
    // BCn helpers
    // ------------------------------------------------------------
    static void rgb565To888(std::uint16_t c, std::uint8_t& r, std::uint8_t& g, std::uint8_t& b)
    {
        const std::uint8_t r5 = (std::uint8_t)((c >> 11) & 0x1F);
        const std::uint8_t g6 = (std::uint8_t)((c >> 5)  & 0x3F);
        const std::uint8_t b5 = (std::uint8_t)((c >> 0)  & 0x1F);

        r = (std::uint8_t)((r5 * 255u + 15u) / 31u);
        g = (std::uint8_t)((g6 * 255u + 31u) / 63u);
        b = (std::uint8_t)((b5 * 255u + 15u) / 31u);
    }

    static void decodeBC1Block(const std::uint8_t* block, std::uint8_t colors[4][4], bool& hasTransparent)
    {
        const std::uint16_t c0 = (std::uint16_t)(block[0] | (block[1] << 8));
        const std::uint16_t c1 = (std::uint16_t)(block[2] | (block[3] << 8));

        std::uint8_t r0, g0, b0, r1, g1, b1;
        rgb565To888(c0, r0, g0, b0);
        rgb565To888(c1, r1, g1, b1);

        colors[0][0] = r0; colors[0][1] = g0; colors[0][2] = b0; colors[0][3] = 255;
        colors[1][0] = r1; colors[1][1] = g1; colors[1][2] = b1; colors[1][3] = 255;

        hasTransparent = false;

        if (c0 > c1)
        {
            // 4-color mode
            colors[2][0] = (std::uint8_t)((2 * r0 + r1) / 3);
            colors[2][1] = (std::uint8_t)((2 * g0 + g1) / 3);
            colors[2][2] = (std::uint8_t)((2 * b0 + b1) / 3);
            colors[2][3] = 255;

            colors[3][0] = (std::uint8_t)((r0 + 2 * r1) / 3);
            colors[3][1] = (std::uint8_t)((g0 + 2 * g1) / 3);
            colors[3][2] = (std::uint8_t)((b0 + 2 * b1) / 3);
            colors[3][3] = 255;
        }
        else
        {
            // 3-color mode (color3 = transparent)
            colors[2][0] = (std::uint8_t)((r0 + r1) / 2);
            colors[2][1] = (std::uint8_t)((g0 + g1) / 2);
            colors[2][2] = (std::uint8_t)((b0 + b1) / 2);
            colors[2][3] = 255;

            colors[3][0] = 0;
            colors[3][1] = 0;
            colors[3][2] = 0;
            colors[3][3] = 0;
            hasTransparent = true;
        }
    }

    static void decodeBC2Alpha(const std::uint8_t* block, std::uint8_t alphaOut[16])
    {
        // 8 bytes alpha, 4 bits per pixel (little endian)
        for (int i = 0; i < 16; ++i)
        {
            const int nibbleIndex = i; // 0..15
            const int byteIndex = nibbleIndex / 2;
            const bool high = (nibbleIndex % 2) == 1;

            const std::uint8_t v = block[byteIndex];
            const std::uint8_t a4 = high ? (v >> 4) : (v & 0x0F);
            alphaOut[i] = (std::uint8_t)((a4 * 255u + 7u) / 15u);
        }
    }

    static void decodeBC3Alpha(const std::uint8_t* block, std::uint8_t alphaOut[16])
    {
        const std::uint8_t a0 = block[0];
        const std::uint8_t a1 = block[1];

        std::uint8_t table[8];
        table[0] = a0;
        table[1] = a1;

        if (a0 > a1)
        {
            table[2] = (std::uint8_t)((6 * a0 + 1 * a1) / 7);
            table[3] = (std::uint8_t)((5 * a0 + 2 * a1) / 7);
            table[4] = (std::uint8_t)((4 * a0 + 3 * a1) / 7);
            table[5] = (std::uint8_t)((3 * a0 + 4 * a1) / 7);
            table[6] = (std::uint8_t)((2 * a0 + 5 * a1) / 7);
            table[7] = (std::uint8_t)((1 * a0 + 6 * a1) / 7);
        }
        else
        {
            table[2] = (std::uint8_t)((4 * a0 + 1 * a1) / 5);
            table[3] = (std::uint8_t)((3 * a0 + 2 * a1) / 5);
            table[4] = (std::uint8_t)((2 * a0 + 3 * a1) / 5);
            table[5] = (std::uint8_t)((1 * a0 + 4 * a1) / 5);
            table[6] = 0;
            table[7] = 255;
        }

        // 48-bit indices, little endian
        std::uint64_t bits = 0;
        for (int i = 0; i < 6; ++i)
            bits |= (std::uint64_t)block[2 + i] << (8 * i);

        for (int i = 0; i < 16; ++i)
        {
            const std::uint32_t idx = (std::uint32_t)((bits >> (3 * i)) & 0x7u);
            alphaOut[i] = table[idx];
        }
    }

    static bool decodeBC1ToRGBA8(
        const std::vector<std::uint8_t>& src,
        std::size_t pixelDataOffset,
        int width, int height,
        std::vector<std::uint8_t>& outRGBA,
        std::string* err)
    {
        const int blocksX = (width + 3) / 4;
        const int blocksY = (height + 3) / 4;
        const std::size_t needed = (std::size_t)blocksX * (std::size_t)blocksY * 8u;

        if (pixelDataOffset + needed > src.size())
        {
            if (err) *err = "DDS: insufficient data for BC1/DXT1 blocks.";
            return false;
        }

        outRGBA.assign((std::size_t)width * (std::size_t)height * 4u, 0);

        const std::uint8_t* p = src.data() + pixelDataOffset;

        for (int by = 0; by < blocksY; ++by)
        {
            for (int bx = 0; bx < blocksX; ++bx)
            {
                const std::uint8_t* block = p + (by * blocksX + bx) * 8u;

                std::uint8_t colors[4][4];
                bool hasTransparent = false;
                decodeBC1Block(block, colors, hasTransparent);

                const std::uint32_t indices =
                    (std::uint32_t)block[4] |
                    ((std::uint32_t)block[5] << 8) |
                    ((std::uint32_t)block[6] << 16) |
                    ((std::uint32_t)block[7] << 24);

                for (int py = 0; py < 4; ++py)
                {
                    for (int px = 0; px < 4; ++px)
                    {
                        const int x = bx * 4 + px;
                        const int y = by * 4 + py;
                        if (x >= width || y >= height)
                            continue;

                        const int idx = (indices >> (2 * (py * 4 + px))) & 0x3;
                        const std::size_t o = ((std::size_t)y * (std::size_t)width + (std::size_t)x) * 4u;

                        outRGBA[o + 0] = colors[idx][0];
                        outRGBA[o + 1] = colors[idx][1];
                        outRGBA[o + 2] = colors[idx][2];
                        outRGBA[o + 3] = colors[idx][3];
                    }
                }
            }
        }

        return true;
    }

    static bool decodeBC2ToRGBA8(
        const std::vector<std::uint8_t>& src,
        std::size_t pixelDataOffset,
        int width, int height,
        std::vector<std::uint8_t>& outRGBA,
        std::string* err)
    {
        const int blocksX = (width + 3) / 4;
        const int blocksY = (height + 3) / 4;
        const std::size_t needed = (std::size_t)blocksX * (std::size_t)blocksY * 16u;

        if (pixelDataOffset + needed > src.size())
        {
            if (err) *err = "DDS: insufficient data for BC2/DXT3 blocks.";
            return false;
        }

        outRGBA.assign((std::size_t)width * (std::size_t)height * 4u, 0);

        const std::uint8_t* p = src.data() + pixelDataOffset;

        for (int by = 0; by < blocksY; ++by)
        {
            for (int bx = 0; bx < blocksX; ++bx)
            {
                const std::uint8_t* block = p + (by * blocksX + bx) * 16u;

                std::uint8_t alpha[16];
                decodeBC2Alpha(block, alpha);

                std::uint8_t colors[4][4];
                bool hasTransparent = false;
                decodeBC1Block(block + 8, colors, hasTransparent);

                const std::uint32_t indices =
                    (std::uint32_t)block[12] |
                    ((std::uint32_t)block[13] << 8) |
                    ((std::uint32_t)block[14] << 16) |
                    ((std::uint32_t)block[15] << 24);

                for (int py = 0; py < 4; ++py)
                {
                    for (int px = 0; px < 4; ++px)
                    {
                        const int x = bx * 4 + px;
                        const int y = by * 4 + py;
                        if (x >= width || y >= height)
                            continue;

                        const int i = py * 4 + px;
                        const int idx = (indices >> (2 * i)) & 0x3;

                        const std::size_t o = ((std::size_t)y * (std::size_t)width + (std::size_t)x) * 4u;
                        outRGBA[o + 0] = colors[idx][0];
                        outRGBA[o + 1] = colors[idx][1];
                        outRGBA[o + 2] = colors[idx][2];
                        outRGBA[o + 3] = alpha[i];
                    }
                }
            }
        }

        return true;
    }

    static bool decodeBC3ToRGBA8(
        const std::vector<std::uint8_t>& src,
        std::size_t pixelDataOffset,
        int width, int height,
        std::vector<std::uint8_t>& outRGBA,
        std::string* err)
    {
        const int blocksX = (width + 3) / 4;
        const int blocksY = (height + 3) / 4;
        const std::size_t needed = (std::size_t)blocksX * (std::size_t)blocksY * 16u;

        if (pixelDataOffset + needed > src.size())
        {
            if (err) *err = "DDS: insufficient data for BC3/DXT5 blocks.";
            return false;
        }

        outRGBA.assign((std::size_t)width * (std::size_t)height * 4u, 0);

        const std::uint8_t* p = src.data() + pixelDataOffset;

        for (int by = 0; by < blocksY; ++by)
        {
            for (int bx = 0; bx < blocksX; ++bx)
            {
                const std::uint8_t* block = p + (by * blocksX + bx) * 16u;

                std::uint8_t alpha[16];
                decodeBC3Alpha(block, alpha);

                std::uint8_t colors[4][4];
                bool hasTransparent = false;
                decodeBC1Block(block + 8, colors, hasTransparent);

                const std::uint32_t indices =
                    (std::uint32_t)block[12] |
                    ((std::uint32_t)block[13] << 8) |
                    ((std::uint32_t)block[14] << 16) |
                    ((std::uint32_t)block[15] << 24);

                for (int py = 0; py < 4; ++py)
                {
                    for (int px = 0; px < 4; ++px)
                    {
                        const int x = bx * 4 + px;
                        const int y = by * 4 + py;
                        if (x >= width || y >= height)
                            continue;

                        const int i = py * 4 + px;
                        const int idx = (indices >> (2 * i)) & 0x3;

                        const std::size_t o = ((std::size_t)y * (std::size_t)width + (std::size_t)x) * 4u;
                        outRGBA[o + 0] = colors[idx][0];
                        outRGBA[o + 1] = colors[idx][1];
                        outRGBA[o + 2] = colors[idx][2];
                        outRGBA[o + 3] = alpha[i];
                    }
                }
            }
        }

        return true;
    }

    static bool decodeBC4ToRGBA8(
        const std::vector<std::uint8_t>& src,
        std::size_t pixelDataOffset,
        int width, int height,
        std::vector<std::uint8_t>& outRGBA,
        std::string* err)
    {
        // BC4: 8 bytes per 4x4 block, same alpha scheme as BC3 alpha
        const int blocksX = (width + 3) / 4;
        const int blocksY = (height + 3) / 4;
        const std::size_t needed = (std::size_t)blocksX * (std::size_t)blocksY * 8u;

        if (pixelDataOffset + needed > src.size())
        {
            if (err) *err = "DDS: insufficient data for BC4 blocks.";
            return false;
        }

        outRGBA.assign((std::size_t)width * (std::size_t)height * 4u, 0);

        const std::uint8_t* p = src.data() + pixelDataOffset;

        for (int by = 0; by < blocksY; ++by)
        {
            for (int bx = 0; bx < blocksX; ++bx)
            {
                const std::uint8_t* block = p + (by * blocksX + bx) * 8u;

                std::uint8_t a[16];
                decodeBC3Alpha(block, a); // works: same index encoding, only 8 bytes total

                for (int py = 0; py < 4; ++py)
                {
                    for (int px = 0; px < 4; ++px)
                    {
                        const int x = bx * 4 + px;
                        const int y = by * 4 + py;
                        if (x >= width || y >= height)
                            continue;

                        const int i = py * 4 + px;
                        const std::size_t o = ((std::size_t)y * (std::size_t)width + (std::size_t)x) * 4u;

                        outRGBA[o + 0] = a[i]; // R
                        outRGBA[o + 1] = 0;
                        outRGBA[o + 2] = 0;
                        outRGBA[o + 3] = 255;
                    }
                }
            }
        }

        return true;
    }

    static bool decodeBC5ToRGBA8(
        const std::vector<std::uint8_t>& src,
        std::size_t pixelDataOffset,
        int width, int height,
        std::vector<std::uint8_t>& outRGBA,
        std::string* err)
    {
        // BC5: two BC4 blocks back-to-back (R and G), 16 bytes per block
        const int blocksX = (width + 3) / 4;
        const int blocksY = (height + 3) / 4;
        const std::size_t needed = (std::size_t)blocksX * (std::size_t)blocksY * 16u;

        if (pixelDataOffset + needed > src.size())
        {
            if (err) *err = "DDS: insufficient data for BC5 blocks.";
            return false;
        }

        outRGBA.assign((std::size_t)width * (std::size_t)height * 4u, 0);

        const std::uint8_t* p = src.data() + pixelDataOffset;

        for (int by = 0; by < blocksY; ++by)
        {
            for (int bx = 0; bx < blocksX; ++bx)
            {
                const std::uint8_t* block = p + (by * blocksX + bx) * 16u;

                std::uint8_t r[16];
                std::uint8_t g[16];
                decodeBC3Alpha(block + 0, r);
                decodeBC3Alpha(block + 8, g);

                for (int py = 0; py < 4; ++py)
                {
                    for (int px = 0; px < 4; ++px)
                    {
                        const int x = bx * 4 + px;
                        const int y = by * 4 + py;
                        if (x >= width || y >= height)
                            continue;

                        const int i = py * 4 + px;
                        const std::size_t o = ((std::size_t)y * (std::size_t)width + (std::size_t)x) * 4u;

                        outRGBA[o + 0] = r[i];
                        outRGBA[o + 1] = g[i];
                        outRGBA[o + 2] = 0;
                        outRGBA[o + 3] = 255;
                    }
                }
            }
        }

        return true;
    }

    // ------------------------------------------------------------
    // Main decode
    // ------------------------------------------------------------
    bool DdsDecoder::decode(DecodedImageData& out, const BinaryData& inBytes, std::string* outError)
    {
        out = DecodedImageData{};

        const auto& buf = inBytes.bytes;
        if (buf.size() < 4 + sizeof(DDS_HEADER))
        {
            if (outError) *outError = "DDS: file too small.";
            return false;
        }

        std::uint32_t magic = 0;
        if (!readU32At(buf, 0, magic) || magic != FCC('D','D','S',' '))
        {
            if (outError) *outError = "DDS: missing magic 'DDS '.";
            return false;
        }

        DDS_HEADER hdr{};
        if (!readAt(buf, 4, hdr))
        {
            if (outError) *outError = "DDS: failed to read header.";
            return false;
        }

        if (hdr.size != 124 || hdr.ddspf.size != 32)
        {
            if (outError) *outError = "DDS: invalid header sizes.";
            return false;
        }

        if (hdr.width == 0 || hdr.height == 0)
        {
            if (outError) *outError = "DDS: invalid dimensions.";
            return false;
        }

        out.width  = (int)hdr.width;
        out.height = (int)hdr.height;

        if (hdr.mipMapCount > 0)
            out.mipCount = (int)hdr.mipMapCount;

        std::size_t pixelDataOffset = 4 + sizeof(DDS_HEADER);

        // Determine compression / format
        bool isFourCC = (hdr.ddspf.flags & DDPF_FOURCC) != 0;
        std::uint32_t fourCC = hdr.ddspf.fourCC;

        // DX10 header handling
        if (isFourCC && fourCC == FCC('D','X','1','0'))
        {
            DDS_HEADER_DXT10 dx10{};
            if (!readAt(buf, pixelDataOffset, dx10))
            {
                if (outError) *outError = "DDS: DX10 header present but missing.";
                return false;
            }
            pixelDataOffset += sizeof(DDS_HEADER_DXT10);

            std::uint32_t mapped = 0;
            if (!dxgiToKind(dx10.dxgiFormat, mapped))
            {
                if (outError) *outError = "DDS: unsupported DXGI_FORMAT in DX10 header: " + std::to_string(dx10.dxgiFormat);
                return false;
            }
            fourCC = mapped;
        }

        // Decode
        if (isFourCC)
        {
            // DXTn / BCn
            if (fourCC == FCC('D','X','T','1'))
            {
                return decodeBC1ToRGBA8(buf, pixelDataOffset, out.width, out.height, out.rgba, outError);
            }
            if (fourCC == FCC('D','X','T','3'))
            {
                return decodeBC2ToRGBA8(buf, pixelDataOffset, out.width, out.height, out.rgba, outError);
            }
            if (fourCC == FCC('D','X','T','5'))
            {
                return decodeBC3ToRGBA8(buf, pixelDataOffset, out.width, out.height, out.rgba, outError);
            }
            if (fourCC == FCC('D','X','T','2'))
            {
                out.premultipliedAlpha = true;
                return decodeBC2ToRGBA8(buf, pixelDataOffset, out.width, out.height, out.rgba, outError);
            }
            if (fourCC == FCC('D','X','T','4'))
            {
                out.premultipliedAlpha = true;
                return decodeBC3ToRGBA8(buf, pixelDataOffset, out.width, out.height, out.rgba, outError);
            }

            // BC4/BC5 (common aliases)
            if (fourCC == FCC('A','T','I','1') || fourCC == FCC('B','C','4','U') || fourCC == FCC('B','C','4','S'))
            {
                return decodeBC4ToRGBA8(buf, pixelDataOffset, out.width, out.height, out.rgba, outError);
            }
            if (fourCC == FCC('A','T','I','2') || fourCC == FCC('B','C','5','U') || fourCC == FCC('B','C','5','S'))
            {
                return decodeBC5ToRGBA8(buf, pixelDataOffset, out.width, out.height, out.rgba, outError);
            }

            if (outError)
            {
                char a = (char)(fourCC & 0xFF);
                char b = (char)((fourCC >> 8) & 0xFF);
                char c = (char)((fourCC >> 16) & 0xFF);
                char d = (char)((fourCC >> 24) & 0xFF);
                *outError = std::string("DDS: unsupported FourCC: '") + a + b + c + d + "'";
            }
            return false;
        }
        else
        {
            // Uncompressed RGB(A) via masks
            return decodeUncompressedToRGBA8(buf, pixelDataOffset, out.width, out.height, hdr.ddspf, out.rgba, outError);
        }
    }
}
