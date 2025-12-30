#pragma once

#include <filesystem>
#include <string>

#include "data/asset/normalized/NormalizedMesh.h" // <- an deinen Pfad anpassen

namespace asset::writer
{
    class ModelWriter
    {
    public:
        static bool writeGlb(
            const asset::normalized::NormalizedMesh& mesh,
            const std::filesystem::path& outFile,
            std::string* outError = nullptr
        );
    };
}
