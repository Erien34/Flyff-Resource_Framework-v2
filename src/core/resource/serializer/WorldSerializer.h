#pragma once
#include "resource/serializer/base/SerializerBase.h"
#include "resource/canonical/rawWorldData.h"

namespace modules::serializer
{
class WorldSerializer final : public SerializerBase
{
public:
    std::string moduleId() const override { return ""; }
    std::string outputModel() const override { return "rawWorldData"; }

protected:
    void serialize(const std::vector<data::TokenData>& streams) override;

private:
    data::module::rawworld::rawWorldData m_data;
};
}
