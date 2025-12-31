#pragma once

#include <string>

namespace asset
{
    struct AssetRecord;
    struct AnimationSource;

    class AnimationLoader
    {
    public:
        static bool load(AnimationSource& out, const AssetRecord& rec, std::string* outError = nullptr);
    };
}
