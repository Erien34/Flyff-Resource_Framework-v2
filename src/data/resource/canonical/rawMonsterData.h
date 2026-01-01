#pragma once
#include <vector>
#include "resource/parse/TokenData.h"

namespace data::module::rawmonster
{
struct rawMonsterData
{
    std::vector<data::TokenData> streams;
    bool valid = false;
};
}
