#pragma once
#include <vector>
#include "TokenData.h"

namespace data::module::quests
{
struct QuestData
{
    std::vector<data::TokenData> streams;
    bool valid = false;
};
}
