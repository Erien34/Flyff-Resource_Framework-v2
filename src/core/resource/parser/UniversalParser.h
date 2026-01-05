#pragma once

#include <string>
#include "resource/path/FileEntry.h"
#include "resource/parse/TokenData.h"
#include "resource/descriptors/Descriptor.h"

namespace modules::parser
{

class UniversalParser
{
public:
    bool isParsable(const FileEntry& file) const;
    data::TokenData parse(
        const FileEntry& file,
        const std::string& resourceRoot
        );
};

}
