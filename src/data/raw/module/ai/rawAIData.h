#pragma once
#include <vector>
#include "TokenData.h"

namespace data::module::rawai
{
struct rawAIData
{
    std::vector<data::TokenData> streams;
    bool valid = false;
};
}
