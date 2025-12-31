#pragma once

#include <string>

namespace asset
{
    struct AssetRecord;
    struct SfxSource;

    class SfxLoader
    {
    public:
        static bool load(SfxSource& out, const AssetRecord& rec, std::string* outError = nullptr);
    };
}
