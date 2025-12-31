#include "core/asset/loader/ImageLoader.h"

#include "index/AssetIndexBuilder.h"          // AssetRecord / AssetTechKind
#include "data/asset/source/ImageSource.h"
#include "data/asset/source/AssetSourceBase.h"

#include <fstream>

namespace fs = std::filesystem;

namespace asset
{
    static std::string toLowerLocal(std::string s)
    {
        for (char& c : s)
            c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
        return s;
    }

    static bool readAllBytes(const fs::path& p, BinaryData& out, std::string* err)
    {
        std::ifstream f(p, std::ios::binary);
        if (!f)
        {
            if (err) *err = "Cannot open file: " + p.string();
            return false;
        }

        f.seekg(0, std::ios::end);
        const auto size = static_cast<size_t>(f.tellg());
        f.seekg(0, std::ios::beg);

        out.bytes.resize(size);
        if (size > 0)
            f.read(reinterpret_cast<char*>(out.bytes.data()), static_cast<std::streamsize>(size));

        if (!f.good() && size > 0)
        {
            if (err) *err = "Failed reading file: " + p.string();
            return false;
        }

        return true;
    }

    static ImageFormat extToFormat(const std::string& extLower)
    {
        if (extLower == ".dds") return ImageFormat::DDS;
        if (extLower == ".tga") return ImageFormat::TGA;
        if (extLower == ".png") return ImageFormat::PNG;
        if (extLower == ".jpg" || extLower == ".jpeg") return ImageFormat::JPG;
        if (extLower == ".bmp") return ImageFormat::BMP;
        return ImageFormat::Unknown;
    }

    bool ImageLoader::load(ImageSource& out, const AssetRecord& rec, std::string* outError)
    {
        out = ImageSource{}; // reset

        out.relPath = rec.relPath;
        out.outPath = rec.outPath;
        out.techKind = rec.techKind;

        // Prefer client path (original), fallback to resource path
        fs::path srcPath;
        if (rec.existsInClient && !rec.clientPath.empty())
            srcPath = rec.clientPath;
        else if (rec.existsInResource && !rec.resourcePath.empty())
            srcPath = rec.resourcePath;
        else
        {
            out.state = AssetState::Error;
            out.errorMessage = "No source path present (client/resource missing)";
            if (outError) *outError = out.errorMessage;
            return false;
        }

        out.sourcePath = srcPath;

        const std::string extLower = toLowerLocal(srcPath.extension().string());
        out.format = extToFormat(extLower);

        std::string err;
        if (!readAllBytes(srcPath, out.bytes, &err))
        {
            out.state = AssetState::Error;
            out.errorMessage = err;
            if (outError) *outError = err;
            return false;
        }

        out.state = AssetState::Loaded;
        return true;
    }
}
