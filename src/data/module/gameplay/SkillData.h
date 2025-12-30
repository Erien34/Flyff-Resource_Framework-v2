#pragma once
#include <vector>
#include "TokenData.h"

namespace data::module::skills
{
struct SkillData
{
    std::vector<data::TokenData> streams;
    bool valid = false;
};
}
