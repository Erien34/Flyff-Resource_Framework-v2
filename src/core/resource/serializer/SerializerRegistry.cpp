#include "SerializerRegistry.h"
#include "Log.h"

using namespace modules::serializer;

void SerializerRegistry::registerSerializer(
    std::unique_ptr<SerializerBase> serializer)
{
    const std::string id = serializer->moduleId();

    if (m_serializers.contains(id))
    {
        core::Log::warn("Serializer already registered: " + id);
        return;
    }

    m_serializers[id] = std::move(serializer);
    //core::Log::info("Serializer registered: " + id);
}

SerializerBase* SerializerRegistry::get(const std::string& moduleId)
{
    auto it = m_serializers.find(moduleId);
    if (it == m_serializers.end())
        return nullptr;
    return it->second.get();
}

const std::unordered_map<std::string,
                         std::unique_ptr<SerializerBase>>&
SerializerRegistry::all() const
{
    return m_serializers;
}
