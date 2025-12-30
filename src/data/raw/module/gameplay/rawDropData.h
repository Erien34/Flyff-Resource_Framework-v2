#pragma once
#include <vector>
#include "TokenData.h"

namespace data::module::rawdrops
{
struct rawDropData
{
    std::vector<data::TokenData> streams;
    bool valid = false;
};
}
