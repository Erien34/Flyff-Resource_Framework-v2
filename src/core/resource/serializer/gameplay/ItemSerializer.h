#pragma once
#include "resource/serializer/base/SerializerBase.h"
#include "gameplay/rawItemData.h"

namespace modules::serializer
{
class ItemSerializer final : public SerializerBase
{
public:
    std::string moduleId() const override { return "items"; }
    std::string outputModel() const override { return "rawItemData"; }

protected:
    void serialize(const std::vector<data::TokenData>& streams) override;

private:
    data::module::rawitems::rawItemData m_data;
};
}
