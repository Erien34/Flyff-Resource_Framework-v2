#pragma once
#include <vector>
#include "resource/parse/TokenData.h"

namespace data::module::rawjobs
{
struct rawJobData
{
    std::vector<data::TokenData> streams;
    bool valid = false;
};
}
