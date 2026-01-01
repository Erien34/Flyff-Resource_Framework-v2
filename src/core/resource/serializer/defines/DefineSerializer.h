#pragma once
#include "resource/serializer/base/SerializerBase.h"
#include "resource/canonical/rawDefineData.h"
#include "resource/parse/TokenData.h"

namespace modules::serializer
{

class DefineSerializer final : public SerializerBase
{
public:
    std::string moduleId() const override { return "defines"; }
    std::string outputModel() const override { return "RawDefineData"; }

protected:
    void serialize(const std::vector<data::TokenData>& streams) override;

private:
    data::raw::rawdefines::RawDefineData m_data;

    void processToken(
        const data::TokenData& stream,
        const data::Token& token,
        std::string& currentEnum
        );
};

} // namespace modules::serializer
