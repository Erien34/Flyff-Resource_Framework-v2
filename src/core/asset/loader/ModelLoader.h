#pragma once

#include <string>

namespace resource
{
    struct AssetRecord;
    struct ModelSource;

    class ModelLoader
    {
    public:
        static bool load(ModelSource& out, const AssetRecord& rec, std::string* outError = nullptr);
    };
}
