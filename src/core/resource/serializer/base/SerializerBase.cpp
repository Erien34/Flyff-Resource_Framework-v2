#pragma once
#include <string>
#include "SerializerBase.h"

#include "TokenData.h"
#include <cctype>

namespace modules::serializer
{

void SerializerBase::run(const std::vector<data::TokenData>& tokens)
{
    // zentraler Einstiegspunkt
    // bewusst KEINE Logik hier
    serialize(tokens);
}

// ------------------------------------------------------------
// Helper: trim (links + rechts)
// ------------------------------------------------------------
static std::string trimString(const std::string& s)
{
    size_t start = 0;
    while (start < s.size() &&
           std::isspace(static_cast<unsigned char>(s[start])))
    {
        ++start;
    }

    size_t end = s.size();
    while (end > start &&
           std::isspace(static_cast<unsigned char>(s[end - 1])))
    {
        --end;
    }

    return s.substr(start, end - start);
}

// ------------------------------------------------------------
// normalizeWhitespace
//  - Spaces + Tabs → TAB
//  - Mehrere Trenner → 1 TAB
//  - Keine führenden / trailing Trenner
// ------------------------------------------------------------
std::string SerializerBase::normalizeWhitespace(const std::string& line) const
{
    std::string out;
    out.reserve(line.size());

    bool lastWasSeparator = false;

    for (char c : line)
    {
        if (c == ' ' || c == '\t')
        {
            if (!lastWasSeparator)
            {
                out.push_back('\t');   // 🔑 kanonischer Trenner
                lastWasSeparator = true;
            }
        }
        else
        {
            out.push_back(c);
            lastWasSeparator = false;
        }
    }

    // Tabs am Anfang / Ende entfernen
    return trimString(out);
}

// ------------------------------------------------------------
// splitByTab
//  - Erwartet bereits normalisierte Zeile
// ------------------------------------------------------------
std::vector<std::string>
SerializerBase::splitByTab(const std::string& normalized) const
{
    std::vector<std::string> parts;
    std::string current;

    for (char c : normalized)
    {
        if (c == '\t')
        {
            parts.push_back(current);
            current.clear();
        }
        else
        {
            current.push_back(c);
        }
    }

    // letztes Feld
    parts.push_back(current);
    return parts;
}

} // namespace modules::serializer
