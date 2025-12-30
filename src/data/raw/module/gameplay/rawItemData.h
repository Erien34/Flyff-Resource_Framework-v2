#pragma once
#include <vector>
#include "TokenData.h"

namespace data::module::rawitems
{
struct rawItemData
{
    std::vector<data::TokenData> streams;
    bool valid = false;
};
}
