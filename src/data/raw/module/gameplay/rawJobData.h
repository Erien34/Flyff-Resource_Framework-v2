#pragma once
#include <vector>
#include "TokenData.h"

namespace data::module::rawjobs
{
struct rawJobData
{
    std::vector<data::TokenData> streams;
    bool valid = false;
};
}
