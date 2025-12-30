#pragma once

#include "resource/serializer/base/SerializerBase.h"
#include "data/raw/module/assets/rawAssetData.h"

namespace modules::serializer
{

class AssetSerializer final : public SerializerBase
{
public:
    AssetSerializer();

    // MUSS exakt zur Base passen
    void serialize(
        const std::vector<data::TokenData>& streams
        ) override;

    std::string moduleId() const override { return "assets"; }
    std::string outputModel() const override { return "RawAssetData"; }

private:
    data::module::rawasset::RawAssetData m_data;

    bool parseEntryLine(
        const std::string& line,
        data::module::rawasset::RawAssetEntry& out
        ) const;
};

} // namespace modules::serializer
