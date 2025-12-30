#pragma once

#include <vector>

#include "SourceDescriptor.h"

namespace core::source::descriptor
{

class SourceDescriptorRegistry
{
public:
    void clear();

    void registerDescriptor(SourceDescriptor descriptor);

    const std::vector<SourceDescriptor>& descriptors() const;

private:
    std::vector<SourceDescriptor> m_descriptors;
};

} // namespace core::source::descriptor
