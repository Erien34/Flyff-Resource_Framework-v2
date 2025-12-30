#pragma once

#include <string>

#include "SourceDescriptorRegistry.h"

namespace core::source::descriptor
{

class SourceDescriptorLoader
{
public:
    bool loadAll(
        const std::string& directory,
        SourceDescriptorRegistry& outRegistry,
        std::string* error = nullptr
    );
};

} // namespace core::source::descriptor
