#pragma once

#include "DecoderBase.h"
#include "data/asset/decoded/DecodedSfxData.h"
#include "data/asset/source/SfxSource.h"

namespace resource::decode
{
class SfxDecoder final : public DecoderBase
{
public:
    struct Result
    {
        DecodeStatus status;
        DecodedSfxData data;
    };

    // Memory-only decode
    static Result decode(const resource::SfxSource& src);
};
}
