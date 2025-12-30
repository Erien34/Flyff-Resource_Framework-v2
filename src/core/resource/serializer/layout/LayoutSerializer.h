#pragma once
#include "resource/serializer/base/SerializerBase.h"
#include "layout/rawLayoutData.h"

namespace modules::serializer
{
class LayoutSerializer final : public SerializerBase
{
public:
    std::string moduleId() const override { return "layout"; }
    std::string outputModel() const override { return "rawLayoutData"; }

protected:
    void serialize(const std::vector<data::TokenData>& streams) override;

private:
    data::module::rawlayout::rawLayoutData m_data;
};
}
