#pragma once
#include <string>
#include <vector>
#include "resource/parse/TokenData.h"

namespace data::module::rawskills
{

struct RawSkillEntry
{
    int line = 0;
    std::vector<std::string> fields;
};

struct RawSkillStream
{
    std::string moduleId;
    std::string sourceFile;
    std::string domain;

    std::vector<data::Token> rawTokens;   // Snapshot
    std::vector<RawSkillEntry> skills;    // 1 Zeile = 1 Skill-Eintrag
};

struct rawSkillData
{
    bool valid = false;
    std::vector<RawSkillStream> streams;
};

} // namespace data::module::rawskills
