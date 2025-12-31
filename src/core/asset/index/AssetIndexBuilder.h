#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>

namespace asset
{
// -----------------------------
// Semantische Klassifikation (Domain / Use-Case)
// -----------------------------
enum class AssetKind
{
    Unknown = 0,
    Ignore,

    RuntimeChr,

    UiSprite,
    Icon,

    Texture,
    Image,

    Model,
    Animation,
    Skeleton,

    Sound,
    Sfx,
    Effect
};

// -----------------------------
// Technische Klassifikation (Format / Converter-Sicht)
// -----------------------------
enum class AssetTechKind
{
    Unknown = 0,

    Ignore,       // dll/exe/bat/log/zip/... + txt/inc
    RuntimeGlue,  // chr

    Texture,      // dds
    Image,        // tga/png/jpg/jpeg/bmp

    Model,        // o3d/ase
    Animation,    // ani

    Sound,        // wav/ogg/bgm/mp3
    Sfx           // sfx
};

struct AssetRecord
{
    // semantisch (Editor/Resolver)
    AssetKind kind = AssetKind::Unknown;

    // technisch (Converter/Importer)
    AssetTechKind techKind = AssetTechKind::Unknown;

    // relative path inside client root (mirrored)
    std::filesystem::path relPath;

    // absolute paths
    std::filesystem::path clientPath;
    std::filesystem::path resourcePath;

    bool existsInClient   = false;
    bool existsInResource = false;

    // output target in assetRoot (mirrored)
    std::filesystem::path outPath;
    bool existsInOut = false;
};

class AssetIndexBuilder
{
public:
    struct Settings
    {
        std::filesystem::path clientRoot;
        std::filesystem::path resourceRoot;

        // where converted/normalized assets live
        std::filesystem::path assetRoot;

        // resource subfolders of interest
        std::filesystem::path resourceModelRoot; // if empty -> resourceRoot/Model

        // mirror exactly client structure into assetRoot
        bool mirrorClientStructure = true;
    };

    using Index = std::unordered_map<std::string, AssetRecord>;

    AssetIndexBuilder() = default;

    Index build(const Settings& s) const;

    // Semantische Klassifikation (2-stufig intern: tech + domain)
    static AssetKind classifyByPath(const std::filesystem::path& rel);

    // Technische Klassifikation (nur Extension)
    static AssetTechKind detectTechKindByExt(const std::filesystem::path& rel);

    static std::string makeKey(const std::filesystem::path& rel);

private:
    static void scanFolder(
        const std::filesystem::path& root,
        const std::filesystem::path& baseForRel,
        bool isClient,
        Index& io
        );
};

} // namespace asset
