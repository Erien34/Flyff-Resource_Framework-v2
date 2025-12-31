// AssetIndexBuilder is FINAL.
// Any runtime interpretation (.chr, .inc, etc.) must NOT be added here.
// See RuntimeIndexBuilder.
#include "AssetIndexBuilder.h"

#include <algorithm>
#include <string>
#include <cctype>

namespace fs = std::filesystem;

namespace asset
{
static std::string toLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

// -----------------------------
// stable key = lowercase generic path
// -----------------------------
std::string AssetIndexBuilder::makeKey(const fs::path& rel)
{
    return toLower(rel.generic_string());
}

// -----------------------------
// TECH: Extension-only (Converter-Sicht)
// -----------------------------
AssetTechKind AssetIndexBuilder::detectTechKindByExt(const fs::path& rel)
{
    const std::string ext = toLower(rel.extension().string());

    // "Noise" / nicht relevant fürs Scannen
    if (ext == ".dll" || ext == ".exe" || ext == ".bat" || ext == ".log" ||
        ext == ".bin" || ext == ".dat" || ext == ".zip" || ext == ".pak" ||
        ext == ".res" || ext == ".thm")
        return AssetTechKind::Ignore;

    // Script/Data (nicht Asset-Pipeline)
    if (ext == ".txt" || ext == ".inc")
        return AssetTechKind::Ignore;

    // Runtime glue
    if (ext == ".chr" || ext == ".ase")
        return AssetTechKind::RuntimeGlue;

    // 3D
    if (ext == ".o3d")
        return AssetTechKind::Model;

    if (ext == ".ani")
        return AssetTechKind::Animation;

    // sfx
    if (ext == ".sfx")
        return AssetTechKind::Sfx;

    // audio
    if (ext == ".wav" || ext == ".ogg" || ext == ".bgm" || ext == ".mp3")
        return AssetTechKind::Sound;

    // images
    if (ext == ".dds")
        return AssetTechKind::Texture; // starkes Signal: Material/Texture

    if (ext == ".tga" || ext == ".png" || ext == ".jpg" ||
        ext == ".jpeg" || ext == ".bmp")
        return AssetTechKind::Image;

    return AssetTechKind::Unknown;
}

// -----------------------------
// SEMANTIC: Domain-based override (Editor/Resolver Sicht)
// 2-stufig: tech -> semantic
// -----------------------------
AssetKind AssetIndexBuilder::classifyByPath(const fs::path& rel)
{
    const std::string p   = toLower(rel.generic_string());
    const std::string ext = toLower(rel.extension().string());

    const AssetTechKind tech = detectTechKindByExt(rel);

    // 0) Ignore/Glue sofort raus
    if (tech == AssetTechKind::Ignore)
        return AssetKind::Ignore;

    if (tech == AssetTechKind::RuntimeGlue)
        return AssetKind::RuntimeChr;

    // 1) THEME => UI Sprites (egal ob TGA/PNG/DDS/etc)
    if (p.rfind("theme/", 0) == 0)
    {
        if (tech == AssetTechKind::Image || tech == AssetTechKind::Texture)
            return AssetKind::UiSprite;
        return AssetKind::Unknown;
    }

    // 2) ICON => Icons
    if (p.rfind("icon/", 0) == 0 ||
        p.find("/icon/")  != std::string::npos ||
        p.find("/icons/") != std::string::npos)
    {
        if (tech == AssetTechKind::Image || tech == AssetTechKind::Texture)
            return AssetKind::Icon;
        return AssetKind::Unknown;
    }

    // 3) SFX tree
    if (p.rfind("sfx/", 0) == 0)
    {
        if (p.find("sfx/scripts/") != std::string::npos)
            return AssetKind::Ignore;

        if (tech == AssetTechKind::Sfx)
            return AssetKind::Sfx;

        // SFX textures (dds/tga/png/..)
        if (tech == AssetTechKind::Image || tech == AssetTechKind::Texture)
            return AssetKind::Texture;

        // selten, aber möglich: Audio innerhalb SFX
        if (tech == AssetTechKind::Sound)
            return AssetKind::Sound;

        return AssetKind::Unknown;
    }

    // 4) SOUND/MUSIC tree
    if (p.rfind("sound/", 0) == 0 || p.rfind("music/", 0) == 0)
    {
        if (tech == AssetTechKind::Sound)
            return AssetKind::Sound;
        return AssetKind::Unknown;
    }

    // 5) MODEL tree
    if (p.rfind("model/", 0) == 0)
    {
        if (tech == AssetTechKind::Model)
            return AssetKind::Model;

        if (tech == AssetTechKind::Animation || ext == ".ani")
            return AssetKind::Animation;

        if (tech == AssetTechKind::Image || tech == AssetTechKind::Texture)
            return AssetKind::Texture;

        return AssetKind::Unknown;
    }

    // 6) TEXTURE folders (generic)
    if (p.rfind("texture/", 0) == 0 || p.find("/texture/") != std::string::npos)
    {
        if (tech == AssetTechKind::Image || tech == AssetTechKind::Texture)
            return AssetKind::Texture;
        return AssetKind::Unknown;
    }

    // 7) Fallback: semantisch aus tech ableiten (vorsichtig)
    switch (tech)
    {
    case AssetTechKind::Model:     return AssetKind::Model;
    case AssetTechKind::Animation:return AssetKind::Animation;
    case AssetTechKind::Sfx:       return AssetKind::Sfx;
    case AssetTechKind::Sound:     return AssetKind::Sound;
    case AssetTechKind::Texture:   return AssetKind::Texture;
    case AssetTechKind::Image:     return AssetKind::Image;   // wichtig: nicht pauschal Texture!
    default:                       return AssetKind::Unknown;
    }
}

// -----------------------------
// scanFolder: Domain-Filter + record filling
// -----------------------------
void AssetIndexBuilder::scanFolder(
    const fs::path& root,
    const fs::path& baseForRel,
    bool isClient,
    Index& io)
{
    if (root.empty() || !fs::exists(root))
        return;

    for (auto const& it : fs::recursive_directory_iterator(root))
    {
        if (!it.is_regular_file())
            continue;

        const fs::path abs = it.path();
        fs::path rel;
        try { rel = fs::relative(abs, baseForRel); }
        catch (...) { continue; }

        if (rel.empty())
            continue;

        // =====================================================
        // FIX: Resource-Model-Dateien logisch unter "model/..." einordnen
        // =====================================================
        if (!isClient)
        {
            // rel ist z.B. "mvr_xxx.o3d" oder "texture/abc.dds"
            // → erzwinge Namespace "model/..."
            rel = fs::path("model") / rel;
        }

        // =====================================================
        // Domain Filter (Client-Struktur)
        // =====================================================
        auto itRoot = rel.begin();
        if (itRoot == rel.end())
            continue;

        std::string rootFolder = toLower(itRoot->string());

        if (isClient)
        {
            // Client-Domänen (inkl. theme!)
            if (rootFolder != "client" &&
                rootFolder != "char"   &&
                rootFolder != "icon"   &&
                rootFolder != "item"   &&
                rootFolder != "model"  &&
                rootFolder != "music"  &&
                rootFolder != "sfx"    &&
                rootFolder != "sound"  &&
                rootFolder != "theme")
            {
                continue;
            }
        }
        else
        {
            // Resource: nur model/
            if (rootFolder != "model")
                continue;
        }

        // =====================================================
        // Extra Ignore: SFX scripts
        // =====================================================
        const std::string relLower = toLower(rel.generic_string());
        if (relLower.find("sfx/scripts/") != std::string::npos)
            continue;

        // =====================================================
        // Classification (2-stufig: Tech -> Semantic)
        // =====================================================
        AssetTechKind tech = detectTechKindByExt(rel);
        AssetKind kind     = classifyByPath(rel);

        // Asset-Pipeline: Ignore komplett nicht aufnehmen
        if (kind == AssetKind::Ignore)
            continue;

        const std::string key = makeKey(rel);

        auto& rec = io[key];
        rec.relPath  = rel;
        rec.kind     = kind;
        rec.techKind = tech;

        if (isClient)
        {
            rec.clientPath      = abs;
            rec.existsInClient  = true;
        }
        else
        {
            rec.resourcePath     = abs;
            rec.existsInResource = true;
        }
    }
}


// -----------------------------
// build
// -----------------------------
AssetIndexBuilder::Index AssetIndexBuilder::build(const Settings& s) const
{
    Index idx;

    // 1) client
    scanFolder(s.clientRoot, s.clientRoot, true, idx);

    // 2) resource model only
    const fs::path modelRoot = s.resourceModelRoot;
    if (!modelRoot.empty() && fs::exists(modelRoot))
        scanFolder(modelRoot, modelRoot, false, idx);

    // 3) outPath
    for (auto& kv : idx)
    {
        auto& rec = kv.second;
        rec.outPath = s.assetRoot / rec.relPath;
        rec.existsInOut = fs::exists(rec.outPath);
    }

    return idx;
}

} // namespace asset
