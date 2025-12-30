#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "TokenData.h"

namespace data::module::defines
{
struct DefineData
{
    std::vector<data::TokenData> streams;
    std::unordered_map<std::string, std::string> defines;
    bool valid = false;
};
}
