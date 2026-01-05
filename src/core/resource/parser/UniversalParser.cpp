#include "UniversalParser.h"
#include "Log.h"
#include "LineFilter.h"

#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace modules::parser
{

// ------------------------------------------------------------
// helpers
// ------------------------------------------------------------

static std::string toLower(std::string s)
{
    std::transform(
        s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); }
        );
    return s;
}

static bool startsWith(
    const std::vector<uint8_t>& b,
    std::initializer_list<uint8_t> p)
{
    if (b.size() < p.size())
        return false;

    size_t i = 0;
    for (auto v : p)
    {
        if (b[i++] != v)
            return false;
    }
    return true;
}

#ifdef _WIN32
static std::string wideToUtf8(const std::wstring& w)
{
    if (w.empty())
        return {};

    int size = WideCharToMultiByte(
        CP_UTF8, 0,
        w.data(), static_cast<int>(w.size()),
        nullptr, 0, nullptr, nullptr);

    std::string out(size, '\0');
    WideCharToMultiByte(
        CP_UTF8, 0,
        w.data(), static_cast<int>(w.size()),
        out.data(), size, nullptr, nullptr);

    return out;
}
#endif

static std::string bytesToUtf8(
    const std::vector<uint8_t>& bytes,
    std::string* detectedEncoding = nullptr)
{
    if (startsWith(bytes, {0xEF, 0xBB, 0xBF}))
    {
        if (detectedEncoding)
            *detectedEncoding = "utf-8-bom";

        return std::string(
            reinterpret_cast<const char*>(bytes.data() + 3),
            reinterpret_cast<const char*>(bytes.data() + bytes.size()));
    }

    if (startsWith(bytes, {0xFF, 0xFE}))
    {
        if (detectedEncoding)
            *detectedEncoding = "utf-16-le";

#ifdef _WIN32
        const wchar_t* wptr =
            reinterpret_cast<const wchar_t*>(bytes.data() + 2);
        size_t wlen = (bytes.size() - 2) / 2;
        std::wstring w(wptr, wptr + wlen);
        return wideToUtf8(w);
#else
        return std::string(
            reinterpret_cast<const char*>(bytes.data()),
            reinterpret_cast<const char*>(bytes.data() + bytes.size()));
#endif
    }

    if (startsWith(bytes, {0xFE, 0xFF}))
    {
        if (detectedEncoding)
            *detectedEncoding = "utf-16-be";

#ifdef _WIN32
        std::vector<uint8_t> swapped;
        swapped.reserve(bytes.size() - 2);

        for (size_t i = 2; i + 1 < bytes.size(); i += 2)
        {
            swapped.push_back(bytes[i + 1]);
            swapped.push_back(bytes[i]);
        }

        const wchar_t* wptr =
            reinterpret_cast<const wchar_t*>(swapped.data());
        size_t wlen = swapped.size() / 2;
        std::wstring w(wptr, wptr + wlen);
        return wideToUtf8(w);
#else
        return std::string(
            reinterpret_cast<const char*>(bytes.data()),
            reinterpret_cast<const char*>(bytes.data() + bytes.size()));
#endif
    }

    if (detectedEncoding)
        *detectedEncoding = "unknown(assume-utf8/ansi)";

    return std::string(
        reinterpret_cast<const char*>(bytes.data()),
        reinterpret_cast<const char*>(bytes.data() + bytes.size()));
}

static void splitLines(
    const std::string& text,
    std::vector<std::string>& outLines)
{
    std::istringstream ss(text);
    std::string line;

    while (std::getline(ss, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        outLines.push_back(std::move(line));
    }
}

static const char* toStr(FileEntry::Source s)
{
    switch (s)
    {
    case FileEntry::Source::Client:   return "client";
    case FileEntry::Source::Resource: return "resource";
    default:                          return "unknown";
    }
}

// ------------------------------------------------------------
// UniversalParser
// ------------------------------------------------------------

bool UniversalParser::isParsable(const FileEntry& file) const
{
    const std::string ext =
        toLower(std::filesystem::path(file.filename).extension().string());

    return ext == ".txt" ||
           ext == ".inc" ||
           ext == ".csv" ||
           ext == ".h"   ||
           ext == ".hpp" ||
           ext == ".wld" ||
           ext == ".rgn";
}

data::TokenData UniversalParser::parse(
    const FileEntry& file,
    const std::string& /*resourceRoot*/)
{
    data::TokenData out;
    out.sourceFile   = file.filename;
    out.moduleId    = file.moduleId;
    out.domain       = file.domain;
    out.source       = file.source;
    out.absolutePath = file.absolutePath;

    if (!isParsable(file))
        return out;

    std::ifstream in(file.absolutePath, std::ios::binary);
    if (!in.is_open())
    {
        core::Log::error(
            "Parser: Failed to open file: " + file.absolutePath);
        return out;
    }

    std::vector<uint8_t> bytes(
        (std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>());

    std::string utf8 = bytesToUtf8(bytes);

    std::vector<std::string> lines;
    lines.reserve(4096);
    splitLines(utf8, lines);

    int lineNo = 0;
    for (auto& line : lines)
    {
        ++lineNo;
        if (LineFilter::shouldIgnore(line))
            continue;

        data::Token t;
        t.value = std::move(line);
        t.line  = lineNo;
        out.tokens.push_back(std::move(t));
    }

    core::Log::info(
        "Parsed " + file.filename +
        ", domain=" + out.domain +
        ", source=" + std::string(toStr(file.source)) +
        ", tokens=" + std::to_string(out.tokens.size())
        );

    return out;
}

} // namespace modules::parser
