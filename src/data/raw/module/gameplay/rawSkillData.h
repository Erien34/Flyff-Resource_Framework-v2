#pragma once
#include <vector>
#include "TokenData.h"

namespace data::module::rawskills
{
struct rawSkillData
{
    std::vector<data::TokenData> streams;
    bool valid = false;
};
}
