#pragma once

#include <filesystem>
#include <string>
#include <optional>

#include "index/AssetIndexBuilder.h" // für AssetTechKind, AssetKind
#include "BinaryData.h"

namespace resource
{
    enum class AssetState
    {
        Unloaded,
        Loaded,
        Failed,
        Error
    };

    struct AssetSourceBase
    {
        void resetBase()
        {
            errorMessage.clear();
            state = AssetState::Unloaded;
        }

        // Identität
        AssetKind     semanticKind = AssetKind::Unknown;
        AssetTechKind techKind     = AssetTechKind::Unknown;

        // Herkunft
        std::filesystem::path sourcePath;   // client oder resource
        std::filesystem::path relPath;      // aus AssetIndexBuilder
        std::filesystem::path outPath;      // Zielpfad

        // Metadaten
        std::string errorMessage;

        // Lifecycle
        AssetState state = AssetState::Unloaded;

        bool isLoaded() const  { return state == AssetState::Loaded; }
        bool isError() const  { return state == AssetState::Error; }
        bool isFailed() const  { return state == AssetState::Failed; }
    };
}
