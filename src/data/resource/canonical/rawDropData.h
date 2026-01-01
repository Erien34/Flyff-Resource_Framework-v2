#pragma once
#include <vector>
#include "resource/parse/TokenData.h"

namespace data::module::rawdrops
{
struct rawDropData
{
    std::vector<data::TokenData> streams;
    bool valid = false;
};
}
