#pragma once
#include "resource/serializer/base/SerializerBase.h"
#include "resource/canonical/rawTextData.h"
#include "resource/parse/TokenData.h"

namespace modules::serializer
{

class TextSerializer final : public SerializerBase
{
public:
    std::string moduleId() const override { return "text"; }
    std::string outputModel() const override { return "rawTextData"; }

protected:
    void serialize(const std::vector<data::TokenData>& streams) override;

private:
    data::module::rawtext::rawTextData m_data;
};

}
