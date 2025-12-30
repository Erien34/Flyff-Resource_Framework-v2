#pragma once

#include <string>
#include <vector>

namespace data::module::rawasset
{

struct RawAssetEntry
{
    std::string name;
    int objectId = 0;
    std::string modelType;
    std::string part;

    int pick = 0;
    int distance = 0;
    int fly = 0;

    float scale = 1.0f;

    int transparent = 0;
    int shadow = 0;
    int textureEx = 0;
    int value = 0;
};

struct RawAssetBlock
{
    std::string label;

    std::vector<RawAssetEntry> entries;
    std::vector<RawAssetBlock> children;
};

struct RawAssetFile
{
    std::string filename;
    std::vector<RawAssetBlock> roots;
};

struct RawAssetData
{
    std::vector<RawAssetFile> files;
};

} // namespace data::module::rawasset

