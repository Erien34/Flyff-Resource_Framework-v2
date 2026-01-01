#pragma once

#include "resource/parse/TokenData.h"
#include "serializer/base/SerializerBase.h"
#include "data/resource/canonical/rawLayoutData.h"

namespace modules::serializer
{
class LayoutSerializer final : public SerializerBase
{
public:
    std::string moduleId() const override { return "layout"; }

    std::string outputModel() const override
    {
        return "resource.canonical.layout";
    }

protected:
    void serialize(const std::vector<data::TokenData>& streams) override;

private:
    data::resource::canonical::rawLayoutData m_data;
};
}

