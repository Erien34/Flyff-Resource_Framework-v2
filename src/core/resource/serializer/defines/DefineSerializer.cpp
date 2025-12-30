#include "DefineSerializer.h"
#include "Log.h"

using namespace modules::serializer;
using namespace data::raw::rawdefines;

void DefineSerializer::serialize(const std::vector<data::TokenData>& streams)
{
    m_data.files.clear();
    m_data.valid = false;

    for (const auto& stream : streams)
    {
        std::string currentEnum;

        for (const auto& token : stream.tokens)
        {
            processToken(stream, token, currentEnum);
        }
    }

    m_data.valid = !m_data.files.empty();

    core::Log::info(
        "DefineSerializer: collected RAW define data for " +
        std::to_string(m_data.files.size()) +
        " domains"
        );
}

void DefineSerializer::processToken(
    const data::TokenData& stream,
    const data::Token& token,
    std::string& currentEnum)
{
    std::string line = normalizeWhitespace(token.value);
    if (line.empty())
        return;

    auto parts = splitByTab(line);
    if (parts.empty())
        return;

    RawDefineRecord rec{};
    rec.sourceFile = stream.sourceFile;
    rec.line = static_cast<uint32_t>(token.line);

    auto& fileData =
        m_data.files[stream.domain][stream.sourceFile];

    // ----------------------------------
    // #define
    // ----------------------------------
    if (parts[0] == "#define")
    {
        rec.kind = RawDefineKind::MacroDefine;
        if (parts.size() > 1)
            rec.name = parts[1];

        for (size_t i = 2; i < parts.size(); ++i)
        {
            if (!rec.rawValue.empty())
                rec.rawValue += '\t';
            rec.rawValue += parts[i];
        }

        fileData.records.push_back(rec);
        return;
    }

    // ----------------------------------
    // enum / enum class
    // ----------------------------------
    if (parts[0] == "enum")
    {
        rec.kind = RawDefineKind::EnumDecl;

        size_t nameIndex =
            (parts.size() > 2 && parts[1] == "class") ? 2 : 1;

        if (nameIndex < parts.size())
            rec.name = parts[nameIndex];

        currentEnum = rec.name;
        fileData.records.push_back(rec);
        return;
    }

    // ----------------------------------
    // enum end
    // ----------------------------------
    if (parts[0] == "}")
    {
        currentEnum.clear();
        return;
    }

    // ----------------------------------
    // enum member
    // ----------------------------------
    if (!currentEnum.empty())
    {
        rec.kind = RawDefineKind::EnumMember;
        rec.scope = currentEnum;
        rec.name = parts[0];

        if (parts.size() >= 3 && parts[1] == "=")
            rec.rawValue = parts[2];

        fileData.records.push_back(rec);
        return;
    }

    // ----------------------------------
    // const / constexpr
    // ----------------------------------
    if (parts[0] == "const" || parts[0] == "constexpr")
    {
        rec.kind =
            (parts[0] == "constexpr")
                ? RawDefineKind::Constexpr
                : RawDefineKind::Const;

        if (parts.size() >= 3)
        {
            rec.rawType = parts[1];
            rec.name = parts[2];
        }

        fileData.records.push_back(rec);
        return;
    }

    // ----------------------------------
    // other directives
    // ----------------------------------
    if (!parts[0].empty() && parts[0][0] == '#')
    {
        rec.kind = RawDefineKind::Directive;
        rec.rawValue = line;
        fileData.records.push_back(rec);
        return;
    }
}
