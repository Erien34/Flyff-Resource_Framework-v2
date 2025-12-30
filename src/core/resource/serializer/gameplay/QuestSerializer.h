#pragma once
#include "resource/serializer/base/SerializerBase.h"
#include "gameplay/rawQuestData.h"

namespace modules::serializer
{
class QuestSerializer final : public SerializerBase
{
public:
    std::string moduleId() const override { return "quests"; }
    std::string outputModel() const override { return "rawQuestData"; }

protected:
    void serialize(const std::vector<data::TokenData>& streams) override;

private:
    data::module::rawquests::rawQuestData m_data;
};
}
