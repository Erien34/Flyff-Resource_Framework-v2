#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include "base/SerializerBase.h"

namespace modules::serializer
{
class SerializerRegistry
{
public:
    void registerSerializer(std::unique_ptr<SerializerBase> s);
    SerializerBase* get(const std::string& moduleId);
    const std::unordered_map<std::string, std::unique_ptr<SerializerBase>>& all() const;

private:
    std::unordered_map<std::string, std::unique_ptr<SerializerBase>> m_serializers;
};
}
