#pragma once

#include <string>
#include <vector>
#include "resource/parse/TokenData.h"

namespace data::module::rawquest
{

struct RawQuestEntry
{
    int line = 0;
    std::vector<std::string> fields;
};

struct RawQuestBlock
{
    std::string header;
    int headerLine = 0;
    std::vector<RawQuestEntry> body;
};

struct RawQuestStream
{
    std::string moduleId;
    std::string sourceFile;
    std::string domain;

    std::vector<data::Token> rawTokens;

    std::vector<RawQuestEntry> flatEntries;
    std::vector<RawQuestBlock> blocks;
};

struct rawQuestData
{
    bool valid = false;
    std::vector<RawQuestStream> streams;
};

} // namespace data::module::rawquest
