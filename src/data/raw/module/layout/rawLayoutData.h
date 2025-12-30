#pragma once
#include <vector>
#include "TokenData.h"

namespace data::module::rawlayout
{
struct rawLayoutData
{
    std::vector<data::TokenData> streams;
    bool valid = false;
};
}
