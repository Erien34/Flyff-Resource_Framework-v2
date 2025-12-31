#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace asset
{
    struct DecodedAniData
    {
        // Container / Header
        std::string containerKind;     // "ANI", "ANI_LEGACY", "Unknown"
        std::uint32_t version = 0;     // falls erkennbar, sonst 0

        // Referenzen (vorsichtig extrahiert)
        std::vector<std::string> refSkeletons; // bone/skel/o3d refs
        std::vector<std::string> refModels;    // o3d/ase
        std::vector<std::string> refOther;     // sonstige refs mit Punkt

        // Analyse / Debug
        std::vector<std::string> allStrings;

        // Raw unverändert (wichtig für Converter!)
        std::vector<std::uint8_t> raw;
    };
}
