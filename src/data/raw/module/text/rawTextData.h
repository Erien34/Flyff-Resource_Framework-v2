#pragma once
#include <unordered_map>
#include <string>

namespace data::module::rawtext
{

struct RawTextFileData
{
    std::unordered_map<std::string, std::string> entries;
    std::unordered_map<std::string, std::string> namedEntries;
};

struct rawTextData
{
    bool valid = false;

    // domain -> file -> data
    std::unordered_map<
        std::string,
        std::unordered_map<std::string, RawTextFileData>
        > files;
};

}
