#pragma once
#include "Descriptor.h"
#include <unordered_map>
#include <string>

namespace modules::descriptors
{
class DescriptorRegistry
{
public:
    // coreDir = .../descriptors/core
    // pluginDir = .../descriptors/plugins
    bool loadAll(const std::string& coreDir, const std::string& pluginDir);

    const Descriptor* find(const std::string& id) const;
    size_t count() const;

    const std::unordered_map<std::string, Descriptor>& all() const { return m_map; }

private:
    std::unordered_map<std::string, Descriptor> m_map;
};
}
