#pragma once
#include "core/asset/converter/AssetConverterBase.h"
#include "data/asset/source/SfxSource.h"

namespace resource
{
class SfxConverter : public AssetConverterBase
{
public:
    using AssetConverterBase::AssetConverterBase;

    ConvertResult convert(const SfxSource& src) const;
};
} // namespace resource
