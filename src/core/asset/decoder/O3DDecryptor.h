#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace resource
{
class O3DDecryptor
{
public:
    // Decrypt using an explicit key (GameSource-compatible).
    static void decryptWithKey(std::vector<std::uint8_t>& out,
                               const std::vector<std::uint8_t>& in,
                               std::uint8_t key);

    // Try to auto-detect encryption key (0..255).
    // If input already looks decrypted, it returns input unchanged and sets outUsedKey=0xFF.
    // Returns false only if no plausible key was found.
    static bool decryptAuto(std::vector<std::uint8_t>& out,
                            const std::vector<std::uint8_t>& in,
                            std::uint8_t* outUsedKey = nullptr,
                            std::string* outError = nullptr);

private:
    static std::uint8_t decryptByte(std::uint8_t key, std::uint8_t b);
    static bool looksLikeO3DDecrypted(const std::vector<std::uint8_t>& bytes);
    static std::uint32_t readU32LE(const std::vector<std::uint8_t>& b, std::size_t off);
};
}
