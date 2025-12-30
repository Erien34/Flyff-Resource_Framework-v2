#pragma once
#include "resource/serializer/base/SerializerBase.h"
#include "gameplay/rawSkillData.h"

namespace modules::serializer
{
class SkillSerializer final : public SerializerBase
{
public:
    std::string moduleId() const override { return "skills"; }
    std::string outputModel() const override { return "rawSkillData"; }

protected:
    void serialize(const std::vector<data::TokenData>& streams) override;

private:
    data::module::rawskills::rawSkillData m_data;
};
}
