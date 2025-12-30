#pragma once
#include "resource/serializer/base/SerializerBase.h"
#include "gameplay/rawMonsterData.h"

namespace modules::serializer
{
class MonsterSerializer final : public SerializerBase
{
public:
    std::string moduleId() const override { return "monster"; }
    std::string outputModel() const override { return "rawMonsterData"; }

protected:
    void serialize(const std::vector<data::TokenData>& streams) override;

private:
    data::module::rawmonster::rawMonsterData m_data;
};
}
