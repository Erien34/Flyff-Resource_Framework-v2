#pragma once
#include "resource/serializer/base/SerializerBase.h"
#include "gameplay/rawDropData.h"

namespace modules::serializer
{
class DropSerializer final : public SerializerBase
{
public:
    std::string moduleId() const override { return "drops"; }
    std::string outputModel() const override { return "rawDropData"; }

protected:
    void serialize(const std::vector<data::TokenData>& streams) override;

private:
    data::module::rawdrops::rawDropData m_data;
};
}
