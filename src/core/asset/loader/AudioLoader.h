#pragma once

#include <string>

namespace asset
{
    struct AssetRecord;
    struct AudioSource;

    class AudioLoader
    {
    public:
        static bool load(AudioSource& out, const AssetRecord& rec, std::string* outError = nullptr);
    };
}
