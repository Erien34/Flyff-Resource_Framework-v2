#pragma once
#include <vector>
#include "TokenData.h"

namespace data::module::jobs
{
struct JobData
{
    std::vector<data::TokenData> streams;
    bool valid = false;
};
}
