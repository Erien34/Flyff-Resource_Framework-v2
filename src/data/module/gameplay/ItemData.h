#pragma once
#include <vector>
#include "TokenData.h"

namespace data::module::items
{
struct ItemData
{
    std::vector<data::TokenData> streams;
    bool valid = false;
};
}
