#pragma once

#include "data/asset/source/AssetSourceBase.h"
#include "BinaryData.h"

#include <string>

namespace resource
{
    // Technical asset: .ani (FlyFF)
    // The Asset-Pipeline only needs a stable container until conversion.
    struct AnimationSource : public AssetSourceBase
    {
        // Original bytes from disk (so converters/decoders can work on memory)
        BinaryData bytes;

        // Convenience: cached lower-case extension (e.g. ".ani")
        std::string extension;

        // Optional: if you later add an AniDecoder, it can fill this.
        // For now we keep it minimal & compile-safe.
    };
}
