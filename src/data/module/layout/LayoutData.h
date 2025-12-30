#pragma once
#include <vector>
#include "TokenData.h"

namespace data::module::layout
{
struct LayoutData
{
    std::vector<data::TokenData> streams;
    bool valid = false;
};
}
