#pragma once
#include "resource/serializer/base/SerializerBase.h"
#include "resource/canonical/rawJobData.h"

namespace modules::serializer
{

class JobSerializer final : public SerializerBase
{
public:
    std::string moduleId() const override { return "job"; }
    std::string outputModel() const override { return "rawJobData"; }

protected:
    void serialize(const std::vector<data::TokenData>& streams) override;

private:
    data::module::rawjobs::rawJobData m_data;
};

} // namespace modules::serializer
