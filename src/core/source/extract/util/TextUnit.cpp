#include "TextUnit.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace core::source::extract::util
{
    static std::string readAllText(const std::filesystem::path& p)
    {
        std::ifstream f(p, std::ios::binary);
        std::ostringstream ss;
        ss << f.rdbuf();
        std::string s = ss.str();
        // normalize CRLF -> LF
        s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
        return s;
    }

    TextUnit TextUnit::load(const std::filesystem::path& abs, const std::filesystem::path& rel)
    {
        TextUnit u;
        u.absPath = abs;
        u.relPath = rel;
        u.content = readAllText(abs);

        u.lineOffsets.clear();
        u.lineOffsets.push_back(0);
        for (std::size_t i = 0; i < u.content.size(); ++i)
            if (u.content[i] == '\n')
                u.lineOffsets.push_back((int)i + 1);

        return u;
    }

    int TextUnit::lineOfOffset(std::size_t off) const
    {
        // upper_bound gives first offset > off => index == line number
        auto it = std::upper_bound(lineOffsets.begin(), lineOffsets.end(), (int)off);
        return (int)(it - lineOffsets.begin());
    }

    std::string TextUnit::snippetAtLine(int line1Based, int ctx) const
    {
        if (line1Based <= 0) return {};
        int idx = line1Based - 1;

        int startLine = std::max(0, idx - ctx);
        int endLine = std::min((int)lineOffsets.size(), idx + ctx + 1);

        int startOff = lineOffsets[startLine];
        int endOff = (endLine < (int)lineOffsets.size()) ? lineOffsets[endLine] : (int)content.size();

        return content.substr((std::size_t)startOff, (std::size_t)(endOff - startOff));
    }

    std::string jsonEscape(std::string_view s)
    {
        std::string out;
        out.reserve(s.size() + 16);
        for (char c : s)
        {
            switch (c)
            {
            case '\\': out += "\\\\"; break;
            case '"':  out += "\\\""; break;
            case '\n': out += "\\n";  break;
            case '\t': out += "\\t";  break;
            case '\r': break;
            default:
                if ((unsigned char)c < 0x20) { /* skip control */ }
                else out += c;
                break;
            }
        }
        return out;
    }
}
