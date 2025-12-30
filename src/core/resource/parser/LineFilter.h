#pragma once
#include <string>

namespace modules::parser
{
class LineFilter
{
public:
    static bool shouldIgnore(const std::string& line);
};
}
