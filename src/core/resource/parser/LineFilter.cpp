#include "LineFilter.h"
#include <algorithm>
#include <cctype>

namespace modules::parser
{

static std::string trim(const std::string& s)
{
    auto notSpace = [](unsigned char c) {
        return !std::isspace(c);
    };

    auto start = std::find_if(s.begin(), s.end(), notSpace);
    auto end   = std::find_if(s.rbegin(), s.rend(), notSpace).base();

    return (start < end) ? std::string(start, end) : std::string();
}

bool LineFilter::shouldIgnore(const std::string& line)
{
    std::string t = trim(line);

    if (t.empty())
        return true;

    if (t.starts_with("//"))
        return true;

    return false;
}

}
