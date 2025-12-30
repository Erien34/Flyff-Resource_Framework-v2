#include "core/asset/loader/AnimationLoader.h"

#include "index/AssetIndexBuilder.h"
#include "data/asset/source/AnimationSource.h"
#include "data/asset/source/AssetSourceBase.h"

#include <fstream>

namespace fs = std::filesystem;

namespace resource
{
    static bool readAllBytes(const fs::path& p, BinaryData& out, std::string* err)
    {
        std::ifstream f(p, std::ios::binary);
        if (!f)
        {
            if (err) *err = "cannot open file: " + p.string();
            return false;
        }

        f.seekg(0, std::ios::end);
        const auto size = static_cast<size_t>(f.tellg());
        f.seekg(0, std::ios::beg);

        out.bytes.resize(size);
        if (size > 0)
        {
            f.read(reinterpret_cast<char*>(out.bytes.data()), static_cast<std::streamsize>(size));
            if (!f)
            {
                if (err) *err = "failed to read file: " + p.string();
                return false;
            }
        }
        return true;
    }

    static std::string toLowerLocal(std::string s)
    {
        for (char& c : s)
            c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
        return s;
    }

    bool AnimationLoader::load(AnimationSource& out, const AssetRecord& rec, std::string* outError)
    {
        out = AnimationSource{};
        out.semanticKind = rec.kind;
        out.techKind = rec.techKind;
        out.relPath = rec.relPath;
        out.outPath = rec.outPath;

        const fs::path input = rec.existsInClient ? rec.clientPath : rec.resourcePath;
        out.sourcePath = input;

        out.extension = toLowerLocal(input.extension().string());

        std::string err;
        if (!readAllBytes(input, out.bytes, &err))
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
