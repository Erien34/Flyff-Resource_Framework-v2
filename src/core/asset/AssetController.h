#pragma once

#include "index/AssetIndexBuilder.h"
#include <filesystem>
#include <string>

namespace resource
{
class AssetController
{
public:
    struct Settings
    {
        AssetIndexBuilder::Settings index;

        // When true, convert/copy only missing outputs
        bool onlyMissing = true;

        // When true, overwrite existing outputs
        bool overwrite = false;
    };

    AssetController() = default;

    bool run(const Settings& s);

private:
    static void ensureDir(const std::filesystem::path& p);

    static std::filesystem::path withExtension(
        const std::filesystem::path& p,
        const std::string& ext
        );

    // bool processOne(const Settings& s, const AssetRecord& rec);

    // // per-kind ops
    // bool doTextureToPng(const std::filesystem::path& src, const std::filesystem::path& dst, std::string* err);
    // bool doImageToPng(const std::filesystem::path& src, const std::filesystem::path& dst, std::string* err);
    // bool doModelToGltf(const std::filesystem::path& src, const std::filesystem::path& dst, std::string* err);
    // bool doCopy(const std::filesystem::path& src, const std::filesystem::path& dst, std::string* err);
    // bool doSfxCopyWithSidecar(const std::filesystem::path& src, const std::filesystem::path& dst, std::string* err);
};

} // namespace resource
