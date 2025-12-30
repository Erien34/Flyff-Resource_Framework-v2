#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace resource
{
    struct BinaryData;

    struct DecodedSfxData
    {
        // "SFX0.1", "SFX0.2", "SFX0.3" oder leer/unknown bei Legacy
        std::string version;

        // Welche "Container-Variante" wir erkannt haben
        // "SFX0x" oder "Legacy"
        std::string containerKind;

        // Extrahierte Referenzen (dedupliziert, lower-case)
        std::vector<std::string> refImages; // dds/tga/png/jpg/jpeg/bmp
        std::vector<std::string> refSounds; // wav/ogg/mp3/bgm
        std::vector<std::string> refModels; // o3d/ase/ani (optional)
        std::vector<std::string> refSfx;    // sfx (verschachtelt)
        std::vector<std::string> refOther;  // alles andere mit Punkt

        // Optional: alle gefundenen "strings" (für Debug/Analyse)
        std::vector<std::string> allStrings;

        // Raw unverändert
        std::vector<std::uint8_t> raw;
    };

    class SfxDecoder
    {
    public:
        static bool decode(DecodedSfxData& out, const BinaryData& inBytes, std::string* outError = nullptr);
    };
}
