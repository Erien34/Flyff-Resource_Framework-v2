#include "AssetConverterBase.h"
#include <fstream>

namespace resource
{
fs::path AssetConverterBase::makeOutPathWithExt(const AssetRecord& rec, const char* newExt) const
{
    fs::path out = rec.outPath;
    out.replace_extension(newExt);
    return out;
}

bool AssetConverterBase::ensureParentDir(const fs::path& outFile, std::string* err) const
{
    std::error_code ec;
    fs::create_directories(outFile.parent_path(), ec);
    if (ec)
    {
        if (err) *err = "create_directories failed: " + outFile.parent_path().string() + " (" + ec.message() + ")";
        return false;
    }
    return true;
}

bool AssetConverterBase::shouldSkipWrite(const fs::path& outFile) const
{
    if (settings().overwriteExisting)
        return false;
    return fs::exists(outFile);
}

bool AssetConverterBase::writeAllBytes(const fs::path& outFile, const std::vector<uint8_t>& bytes, std::string* err) const
{
    if (!ensureParentDir(outFile, err))
        return false;

    std::ofstream f(outFile, std::ios::binary);
    if (!f)
    {
        if (err) *err = "cannot open for write: " + outFile.string();
        return false;
    }
    if (!bytes.empty())
        f.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    return true;
}

bool AssetConverterBase::copyFile(const fs::path& src, const fs::path& dst, std::string* err) const
{
    if (!fs::exists(src))
    {
        if (err) *err = "copyFile: source does not exist: " + src.string();
        return false;
    }
    if (!ensureParentDir(dst, err))
        return false;

    std::error_code ec;
    fs::copy_file(src, dst,
                  settings().overwriteExisting ? fs::copy_options::overwrite_existing
                                              : fs::copy_options::skip_existing,
                  ec);
    if (ec)
    {
        if (err) *err = "copy_file failed: " + src.string() + " -> " + dst.string() + " (" + ec.message() + ")";
        return false;
    }
    return true;
}
} // namespace resource
