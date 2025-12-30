#include "ModelConverter.h"

#include <filesystem>

#include "asset/normalizer/ModelNormalizer.h"      // <- Pfad anpassen
#include "asset/writer/ModelWriter.h"         // <- Pfad anpassen
#include "asset/decoder/O3DDecoder.h"              // <- Pfad anpassen
#include "data/asset/decoded/O3DDecoded.h"
#include "asset/parser/ModelParser.h"
#include "data/asset/parsed/O3DParsed.h"

namespace fs = std::filesystem;

namespace resource
{
    ConvertResult ModelConverter::convert(const ModelSource& src) const
    {
        ConvertResult r;

        // Zielpfad wie gehabt
        fs::path outGlb = src.outPath;
        outGlb.replace_extension(".glb");

        if (shouldSkipWrite(outGlb))
        {
            r.ok = true;
            return r;
        }

        std::string err;
        if (!ensureParentDir(outGlb, &err))
        {
            r.ok = false;
            r.error = err;
            return r;
        }

        // 1) Decode (memory-only aus ModelSource parts)
        resource::O3DDecoded decoded;
        if (!resource::O3DDecoder::decode(decoded, src, &err))
        {
            r.ok = false;
            r.error = err;
            return r;
        }

        // resource::O3DParsed parsed;
        // if (!asset::parser::ModelParser::parse(parsed, decoded, &err))
        // {
        //     r.ok = false;
        //     r.error = err;
        //     return r;
        // }

        // 2) Normalize (DecodedO3D -> NormalizedMesh)
        // asset::normalized::NormalizedMesh nmesh;
        // if (!asset::normalizer::ModelNormalizer::normalizeMesh(nmesh, parsed, &err))
        // {
        //     r.ok = false;
        //     r.error = err;
        //     return r;
        // }

        // 3) Write GLB (NormalizedMesh -> GLB)
        // if (!asset::writer::ModelWriter::writeGlb(nmesh, outGlb, &err))
        // {
        //     r.ok = false;
        //     r.error = err;
        //     return r;
        // }

        r.ok = true;
        return r;
    }
}
