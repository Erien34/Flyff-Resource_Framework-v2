#pragma once

#include <string>

namespace resource
{
    struct AniSource;        // falls du später ein eigenes Source-Modell hast
    struct BinaryData;
    struct DecodedAniData;

    class AniDecoder
    {
    public:
        // Variante A: direkt aus BinaryData (minimal & universell)
        static bool decode(DecodedAniData& out, const BinaryData& bytes, std::string* outError = nullptr);

        // Variante B (optional): aus AniSource
        // static bool decode(DecodedAniData& out, const AniSource& src, std::string* outError = nullptr);
    };
}
