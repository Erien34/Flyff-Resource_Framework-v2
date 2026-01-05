#pragma once
#include <string>
#include <vector>
#include "data/resource/path/FileEntry.h"

namespace data
{
struct Token
{
    std::string type;   // raw, define, identifier, value, symbol, comment
    std::string value;
    int line = 0;
    int column = 1;
};

struct TokenData
{
    std::string moduleId;
    std::string sourceFile;

    std::string domain;
    FileEntry::Source source;
    std::string absolutePath;

    std::vector<Token> tokens;
};
}
