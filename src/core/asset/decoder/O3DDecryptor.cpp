#include "O3DDecryptor.h"

namespace resource
{
static inline std::uint8_t rot4(std::uint8_t v)
{
    // swap nibbles (equivalent to (v<<4)|(v>>4) with uint8 wrap)
    return static_cast<std::uint8_t>((v << 4) | (v >> 4));
}

std::uint8_t O3DDecryptor::decryptByte(std::uint8_t key, std::uint8_t b)
{
    // GameSource: b = ~b ^ key; return (b<<4)|(b>>4);
    std::uint8_t x = static_cast<std::uint8_t>(~b);
    x = static_cast<std::uint8_t>(x ^ key);
    return rot4(x);
}

std::uint32_t O3DDecryptor::readU32LE(const std::vector<std::uint8_t>& b, std::size_t off)
{
    return  (static_cast<std::uint32_t>(b[off + 0])      ) |
           (static_cast<std::uint32_t>(b[off + 1]) <<  8) |
           (static_cast<std::uint32_t>(b[off + 2]) << 16) |
           (static_cast<std::uint32_t>(b[off + 3]) << 24);
}

void O3DDecryptor::decryptWithKey(std::vector<std::uint8_t>& out,
                                  const std::vector<std::uint8_t>& in,
                                  std::uint8_t key)
{
    out.resize(in.size());
    for (std::size_t i = 0; i < in.size(); ++i)
        out[i] = decryptByte(key, in[i]);
}

bool O3DDecryptor::looksLikeO3DDecrypted(const std::vector<std::uint8_t>& bytes)
{
    // Heuristik anhand GameSource Object3D::LoadObject:
    // [0] nameLen (1 byte)
    // [1..nameLen] filename bytes (XOR 0xCD handled by HeaderReader later)
    // next: int32 version (>= VER_MESH == 20)
    if (bytes.size() < 8)
        return false;

    const std::uint8_t nameLen = bytes[0];
    if (nameLen == 0 || nameLen > 63) // GameSource checks cLen>=64 as error
        return false;

    const std::size_t offVer = 1u + static_cast<std::size_t>(nameLen);
    if (offVer + 4u > bytes.size())
        return false;

    const std::uint32_t ver = readU32LE(bytes, offVer);

    // VER_MESH in GameSource: 20
    if (ver < 20u || ver > 200u)
        return false;

    return true;
}

bool O3DDecryptor::decryptAuto(std::vector<std::uint8_t>& out,
                               const std::vector<std::uint8_t>& in,
                               std::uint8_t* outUsedKey,
                               std::string* outError)
{
    out.clear();

    // If it already looks like a decrypted O3D stream, keep it unchanged.
    if (looksLikeO3DDecrypted(in))
    {
        out = in;
        if (outUsedKey) *outUsedKey = 0xFF; // sentinel: "no decryption applied"
        return true;
    }

    // brute-force key 0..255
    std::vector<std::uint8_t> candidate;
    for (int k = 0; k <= 255; ++k)
    {
        decryptWithKey(candidate, in, static_cast<std::uint8_t>(k));
        if (looksLikeO3DDecrypted(candidate))
        {
            out = std::move(candidate);
            if (outUsedKey) *outUsedKey = static_cast<std::uint8_t>(k);
            return true;
        }
    }

    if (outError)
        *outError = "O3DDecryptor::decryptAuto: no plausible key found (0..255).";
    return false;
}
}
