#include "DescriptorRegistry.h"
#include "DescriptorLoader.h"

namespace modules::descriptors
{
bool DescriptorRegistry::loadAll(const std::string& coreDir, const std::string& pluginDir)
{
    m_map.clear();

    // Bootstrap Core falls nötig
    DescriptorLoader::bootstrapCoreIfNeeded(coreDir);

    // Core laden (Pflicht)
    auto core = DescriptorLoader::loadFromDirectory(coreDir, "core");
    if (core.empty())
        return false;

    for (auto& d : core)
        m_map[d.id] = std::move(d);

    // Plugins laden (optional)
    auto plugins = DescriptorLoader::loadFromDirectory(pluginDir, "plugin");
    for (auto& d : plugins)
        m_map[d.id] = std::move(d);

    return true;
}

const Descriptor* DescriptorRegistry::find(const std::string& id) const
{
    auto it = m_map.find(id);
    return it == m_map.end() ? nullptr : &it->second;
}

size_t DescriptorRegistry::count() const
{
    return m_map.size();
}
}
