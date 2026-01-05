#pragma once
#include <string>
#include <vector>
#include "resource/parse/TokenData.h"

namespace data::module::rawjobs
{

struct RawJobEntry
{
    int line = 0;
    std::vector<std::string> fields;
};

struct RawJobStream
{
    std::string moduleId;
    std::string sourceFile;
    std::string domain;

    std::vector<data::Token> rawTokens; // Snapshot
    std::vector<RawJobEntry> jobs;       // 1 Zeile = 1 Job
};

struct rawJobData
{
    bool valid = false;
    std::vector<RawJobStream> streams;
};

} // namespace data::module::rawjobs
