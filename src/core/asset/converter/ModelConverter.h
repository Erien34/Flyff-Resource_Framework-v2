#pragma once
#include "core/asset/converter/AssetConverterBase.h"
#include "data/asset/source/ModelSource.h"

namespace resource
{
    class ModelConverter : public AssetConverterBase
    {
    public:
        using AssetConverterBase::AssetConverterBase;
        ConvertResult convert(const ModelSource& src) const;
    };
}
