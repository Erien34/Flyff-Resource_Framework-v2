#include "core/asset/loader/ModelLoader.h"

#include "index/AssetIndexBuilder.h"
#include "data/asset/source/ModelSource.h"
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
        const std::streamoff size = f.tellg();
        if (size < 0)
        {
            if (err) *err = "invalid size: " + p.string();
            return false;
        }
        f.seekg(0, std::ios::beg);
        out.bytes.resize(static_cast<size_t>(size));
        if (size > 0)
        {
            f.read(reinterpret_cast<char*>(out.bytes.data()), size);
            if (!f)
            {
                if (err) *err = "read failed: " + p.string();
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

    bool ModelLoader::load(ModelSource& out, const AssetRecord& rec, std::string* outError)
    {
        out = ModelSource{}; // reset
        out.relPath = rec.relPath;
        out.sourcePath = rec.existsInClient ? rec.clientPath : rec.resourcePath;
        out.outPath = rec.outPath;
        out.techKind = rec.techKind;
        out.state = AssetState::Unloaded;

        // --- Skeleton from client (preferred) ---
        if (rec.existsInClient)
        {
            out.skeleton.exists = true;
            out.skeleton.path = rec.clientPath;
            out.skeleton.extension = toLowerLocal(rec.clientPath.extension().string());
            std::string err;
            if (!readAllBytes(rec.clientPath, out.skeleton.bytes, &err))
            {
                out.state = AssetState::Error;
                out.errorMessage = err;
                if (outError) *outError = err;
                return false;
            }
        }

        // --- Mesh from resource (preferred) ---
        if (rec.existsInResource)
        {
            out.mesh.exists = true;
            out.mesh.path = rec.resourcePath;
            out.mesh.extension = toLowerLocal(rec.resourcePath.extension().string());
            std::string err;
            if (!readAllBytes(rec.resourcePath, out.mesh.bytes, &err))
            {
                out.state = AssetState::Error;
                out.errorMessage = err;
                if (outError) *outError = err;
                return false;
            }
        }

        // If neither part exists, it's an error
        if (!out.skeleton.exists && !out.mesh.exists)
        {
            const std::string err = "model has no source file (neither client nor resource) for: " + rec.relPath.generic_string();
            out.state = AssetState::Error;
            out.errorMessage = err;
            if (outError) *outError = err;
            return false;
        }

        out.state = AssetState::Loaded;
        return true;
    }
}
