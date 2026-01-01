#pragma once
#include "resource/serializer/base/SerializerBase.h"
#include "resource/canonical/rawAIData.h"

namespace modules::serializer
{
class AiSerializer final : public SerializerBase
{
public:
    std::string moduleId() const override { return "ai"; }
    std::string outputModel() const override { return "rawAiData"; }

protected:
    void serialize(const std::vector<data::TokenData>& streams) override;

private:
    data::module::rawai::rawAIData m_data;
};
}
