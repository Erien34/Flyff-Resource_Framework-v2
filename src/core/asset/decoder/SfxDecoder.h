#pragma once

#include "DecoderBase.h"
#include "data/asset/decoded/DecodedSfxData.h"
#include "data/asset/source/SfxSource.h"

namespace asset::decode
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
    static Result decode(const asset::SfxSource& src);
};
}
