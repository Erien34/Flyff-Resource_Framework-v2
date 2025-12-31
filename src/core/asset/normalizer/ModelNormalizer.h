#pragma once

#include <string>
#include "data/asset/decoded/O3DDecoded.h"
#include "data/asset/normalized/NormalizedMesh.h"
#include "asset/parser/ModelParser.h"
#include "data/asset/parsed/O3DParsed.h"

namespace asset::normalizer
{
    class ModelNormalizer
    {
    public:
        static bool normalizeMesh(
            asset::normalized::NormalizedMesh& out,
            const asset::O3DParsed& src,
            std::string* outError = nullptr
        );
    };
}
