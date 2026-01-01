#pragma once
#include <vector>
#include "resource/parse/TokenData.h"

namespace data::module::rawquests
{
struct rawQuestData
{
    std::vector<data::TokenData> streams;
    bool valid = false;
};
}
