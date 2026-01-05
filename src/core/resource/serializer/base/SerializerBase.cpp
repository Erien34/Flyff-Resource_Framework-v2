#include "SerializerBase.h"

#include "resource/parse/TokenData.h"
#include <cctype>
#include <string>

#include "data/global/GlobalStore.h"

namespace modules::serializer
{

void SerializerBase::run(const std::vector<data::TokenData>& tokens)
{
    serialize(tokens);
}

void SerializerBase::publishModel(const std::string& key, std::any model)
{
    GlobalStore::instance().storeModel(key, std::move(model));
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
                out.push_back('\t');
                lastWasSeparator = true;
            }
        }
        else
        {
            out.push_back(c);
            lastWasSeparator = false;
        }
    }

    return trimString(out);
}

// ------------------------------------------------------------
// splitByTab
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

    parts.push_back(current);
    return parts;
}

bool SerializerBase::shouldIgnoreLine(const std::string& line)
{
    std::string trimmed = normalizeWhitespace(line);

    if (trimmed.empty())
        return true;

    // Block-Kommentar START
    if (!m_inBlockComment)
    {
        auto start = trimmed.find("/*");
        if (start != std::string::npos)
        {
            m_inBlockComment = true;

            // Block beginnt und endet in derselben Zeile
            if (trimmed.find("*/", start + 2) != std::string::npos)
            {
                m_inBlockComment = false;
            }
            return true;
        }
    }
    else
    {
        // Block-Kommentar ENDE
        if (trimmed.find("*/") != std::string::npos)
        {
            m_inBlockComment = false;
        }
        return true;
    }

    return false;
}

bool SerializerBase::isBlockOpen(const std::string& line)
{
    return normalizeWhitespace(line) == "{";
}

bool SerializerBase::isBlockClose(const std::string& line)
{
    return normalizeWhitespace(line) == "}";
}

} // namespace modules::serializer
