#pragma once
#include <string>
#include <vector>
#include "resource/parse/TokenData.h"

namespace data::module::rawdrops
{

struct RawDropEntry
{
    uint32_t line = 0;
    std::vector<std::string> fields;
};

struct RawDropBlock
{
    std::string header;              // QUEST_..., 1, EVENT_...
    uint32_t headerLine = 0;

    std::vector<RawDropEntry> body;  // alles im { ... }
};

struct RawDropStream
{
    std::string moduleId;
    std::string sourceFile;
    std::string domain;

    std::vector<data::Token> rawTokens;

    std::vector<RawDropEntry> flatDrops;   // jede Zeile (verlustfrei)
    std::vector<RawDropBlock> blocks;      // strukturierte Blöcke
};

struct rawDropData
{
    bool valid = false;
    std::vector<RawDropStream> streams;
};

} // namespace data::module::rawdrops
