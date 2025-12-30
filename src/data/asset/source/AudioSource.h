#pragma once

#include "data/asset/source/AssetSourceBase.h"
#include "BinaryData.h"

#include <string>

namespace resource
{
    // Technical asset: wav/ogg/bgm/mp3
    // For now: keep raw bytes (copy/normalize later in converter)
    struct AudioSource : public AssetSourceBase
    {
        BinaryData bytes;
        std::string extension; // lower-case, e.g. ".wav" / ".ogg"
    };
}
