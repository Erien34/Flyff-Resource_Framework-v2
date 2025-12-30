#pragma once
#include <vector>
#include "TokenData.h"

namespace data::module::ai
{
struct AIData
{
    std::vector<data::TokenData> streams;
    bool valid = false;
};
}
