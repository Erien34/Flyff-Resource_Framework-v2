#include "SourceDescriptorRegistry.h"

namespace core::source::descriptor
{

void SourceDescriptorRegistry::clear()
{
    m_descriptors.clear();
}

void SourceDescriptorRegistry::registerDescriptor(SourceDescriptor descriptor)
{
    m_descriptors.emplace_back(std::move(descriptor));
}

const std::vector<SourceDescriptor>&
SourceDescriptorRegistry::descriptors() const
{
    return m_descriptors;
}

} // namespace core::source::descriptor
