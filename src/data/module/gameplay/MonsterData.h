#pragma once
#include <vector>
#include "TokenData.h"

namespace data::module::monster
{
struct MonsterData
{
    std::vector<data::TokenData> streams;
    bool valid = false;
};
}
