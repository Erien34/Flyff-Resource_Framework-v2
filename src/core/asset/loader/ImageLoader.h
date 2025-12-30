#pragma once

#include <string>

namespace resource
{
    struct AssetRecord;
    struct ImageSource;

    class ImageLoader
    {
    public:
        static bool load(ImageSource& out, const AssetRecord& rec, std::string* outError = nullptr);
    };
}
