#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "TokenData.h"

namespace data::module::text
{
struct TextData
{
    std::vector<data::TokenData> streams;
    std::unordered_map<int, std::string> strings;
    bool valid = false;
};
}
