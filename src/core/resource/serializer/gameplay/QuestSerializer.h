#pragma once
#include "resource/serializer/base/SerializerBase.h"
#include "resource/canonical/rawQuestData.h"

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
    data::module::rawquest::rawQuestData m_data;
};
}
