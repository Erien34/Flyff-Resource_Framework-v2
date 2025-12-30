#pragma once

#include <string>

namespace resource
{
    struct ModelSource;
    struct O3DDecoded;

    class O3DDecoder
    {
    public:
        static bool decode(O3DDecoded& out, const ModelSource& src, std::string* outError = nullptr);
    };
}
