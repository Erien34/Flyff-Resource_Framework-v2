#pragma once
#include <vector>
#include "TokenData.h"

namespace data::module::assets
{
struct AssetData
{
    std::vector<data::TokenData> streams;
    bool valid = false;
};
}
