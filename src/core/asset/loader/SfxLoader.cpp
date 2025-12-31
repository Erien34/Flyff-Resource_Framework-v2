#include "core/asset/loader/SfxLoader.h"

#include "index/AssetIndexBuilder.h"
#include "data/asset/source/SfxSource.h"
#include "data/asset/source/AssetSourceBase.h"

#include <fstream>

namespace fs = std::filesystem;

namespace asset
{
    static bool readAllBytes(const fs::path& p, BinaryData& out, std::string* err)
    {
        std::ifstream f(p, std::ios::binary);
        if (!f)
        {
            if (err) *err = "cannot open file: " + p.generic_string();
            return false;
        }

        f.seekg(0, std::ios::end);
        const auto size = static_cast<size_t>(f.tellg());
        f.seekg(0, std::ios::beg);

        out.bytes.resize(size);
        if (size > 0)
            f.read(reinterpret_cast<char*>(out.bytes.data()), static_cast<std::streamsize>(size));

        return true;
    }

    static std::string toLowerLocal(std::string s)
    {
        for (char& c : s)
            c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
        return s;
    }

    bool SfxLoader::load(SfxSource& out, const AssetRecord& rec, std::string* outError)
    {
        out.resetBase();
        out.relPath = rec.relPath;
        out.outPath = rec.outPath;
        out.techKind = rec.techKind;

        // Prefer client path for SFX
        if (rec.existsInClient)
            out.sourcePath = rec.clientPath;
        else if (rec.existsInResource)
            out.sourcePath = rec.resourcePath;
        else
        {
            out.state = AssetState::Failed;
            out.errorMessage = "SfxSource has no input path";
            if (outError) *outError = out.errorMessage;
            return false;
        }

        out.extension = toLowerLocal(out.sourcePath.extension().string());

        std::string err;
        if (!readAllBytes(out.sourcePath, out.bytes, &err))
        {
            out.state = AssetState::Failed;
            out.errorMessage = err;
            if (outError) *outError = err;
            return false;
        }

        out.state = AssetState::Loaded;
        return true;
    }
}
