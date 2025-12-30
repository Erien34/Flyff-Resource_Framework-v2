#pragma once
#include <vector>
#include "TokenData.h"

namespace data::module::drops
{
struct DropData
{
    std::vector<data::TokenData> streams;
    bool valid = false;
};
}
