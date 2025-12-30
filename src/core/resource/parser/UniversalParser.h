#pragma once

#include <string>
#include "FileEntry.h"
#include "TokenData.h"
#include "resource/descriptors/Descriptor.h"

namespace modules::parser
{

class UniversalParser
{
public:
    data::TokenData parse(
        const FileEntry& file,
        const std::string& resourceRoot
        );
};

}
