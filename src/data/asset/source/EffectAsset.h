// WIP: Animation asset data for FlyFF Framework.
#pragma once

#include <string>
#include <filesystem>
#include <vector>
#include <memory>
#include "data/asset/source/AssetSourceBase.h"

namespace asset
{

enum class AnimationFormat
{
    Unknown,
    ANI,        // FlyFF-Animation
    External    // z.B. FBX/GLTF anim
};

struct AnimationClipInfo
{
    std::string name;
    float durationSeconds = 0.0f;
    std::uint32_t keyframeCount = 0;
};

struct AnimationAsset
{
    std::string id;
    std::filesystem::path sourceFile;
    AnimationFormat format = AnimationFormat::Unknown;

    std::vector<AnimationClipInfo> clips;

    bool loadedOk = false;

    static std::string formatToString(AnimationFormat fmt);
};

using AnimationAssetPtr = std::shared_ptr<AnimationAsset>;

} // namespace asset
