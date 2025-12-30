#pragma once

#include "data/asset/source/AssetSourceBase.h"
#include "BinaryData.h"

#include <cstdint>
#include <string>

namespace resource
{
    // Technical asset: .sfx (FlyFF)
    // The Asset-Pipeline keeps it as raw bytes; the Converter/Decoder can later
    // transform it into EffectSeer / JSON / etc.
    struct SfxSource : public AssetSourceBase
    {
        std::string extension;// lower-case file extension (".sfx")

        BinaryData bytes;

        // Optional light-weight meta, if you parse it later.
        uint32_t declaredVersion = 0;
    };
}
