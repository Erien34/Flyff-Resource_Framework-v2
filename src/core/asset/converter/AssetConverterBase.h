#pragma once
#include <filesystem>
#include <string>
#include "core/asset/index/AssetIndexBuilder.h"

namespace fs = std::filesystem;

namespace resource
{
struct ConvertResult
{
    bool ok = false;
    std::string error;
};

struct ConverterSettings
{
    // AssetRoot ist rec.outPath "Root" – wird über AssetIndexBuilder gesetzt.
    // Converter dürfen Extension ersetzen.
    bool overwriteExisting = false;

    // ModelConverter External Tool (optional)
    // Beispiel: "tools/assimp/assimp.exe"
    fs::path modelToolPath;

    // Effekt-Seeker Endung (falls du später sfx->efs oder ähnliches willst)
    // Wenn leer => Endung bleibt wie Input.
    std::string sfxTargetExtension; // z.B. ".efs"
};

class AssetConverterBase
{
public:
    explicit AssetConverterBase(ConverterSettings settings)
        : m_settings(std::move(settings)) {}

    virtual ~AssetConverterBase() = default;

protected:
    const ConverterSettings& settings() const { return m_settings; }

    // Zielpfad aus AssetRecord ableiten + Extension ersetzen
    fs::path makeOutPathWithExt(const AssetRecord& rec, const char* newExt) const;

    // sicherstellen, dass Zielordner existiert (Pipeline legt es zwar an,
    // aber Converter sollten robust sein)
    bool ensureParentDir(const fs::path& outFile, std::string* err) const;

    bool shouldSkipWrite(const fs::path& outFile) const;

    // File utils
    bool writeAllBytes(const fs::path& outFile, const std::vector<uint8_t>& bytes, std::string* err) const;
    bool copyFile(const fs::path& src, const fs::path& dst, std::string* err) const;

private:
    ConverterSettings m_settings;
};
} // namespace resource
