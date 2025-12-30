#include "UniversalParser.h"
#include "Log.h"
#include "LineFilter.h"

#include <fstream>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

static bool startsWith(const std::vector<uint8_t>& b, std::initializer_list<uint8_t> p)
{
    if (b.size() < p.size()) return false;
    size_t i = 0;
    for (auto v : p) { if (b[i++] != v) return false; }
    return true;
}

#ifdef _WIN32
static std::string wideToUtf8(const std::wstring& w)
{
    if (w.empty()) return {};
    int size = WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string out(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), out.data(), size, nullptr, nullptr);
    return out;
}
#endif

static std::string bytesToUtf8(const std::vector<uint8_t>& bytes, std::string* detectedEncoding = nullptr)
{
    if (startsWith(bytes, {0xEF,0xBB,0xBF}))
    {
        if (detectedEncoding) *detectedEncoding = "utf-8-bom";
        return std::string((const char*)bytes.data() + 3, (const char*)bytes.data() + bytes.size());
    }
    if (startsWith(bytes, {0xFF,0xFE}))
    {
        if (detectedEncoding) *detectedEncoding = "utf-16-le";
#ifdef _WIN32
        // bytes -> wchar_t (LE)
        const wchar_t* wptr = (const wchar_t*)(bytes.data() + 2);
        size_t wlen = (bytes.size() - 2) / 2;
        std::wstring w(wptr, wptr + wlen);
        return wideToUtf8(w);
#else
        // fallback: treat as binary if non-windows
        return std::string((const char*)bytes.data(), (const char*)bytes.data() + bytes.size());
#endif
    }
    if (startsWith(bytes, {0xFE,0xFF}))
    {
        if (detectedEncoding) *detectedEncoding = "utf-16-be";
#ifdef _WIN32
        // BE -> LE umdrehen
        std::vector<uint8_t> swapped;
        swapped.reserve(bytes.size() - 2);
        for (size_t i = 2; i + 1 < bytes.size(); i += 2)
        {
            swapped.push_back(bytes[i+1]);
            swapped.push_back(bytes[i]);
        }
        const wchar_t* wptr = (const wchar_t*)swapped.data();
        size_t wlen = swapped.size() / 2;
        std::wstring w(wptr, wptr + wlen);
        return wideToUtf8(w);
#else
        return std::string((const char*)bytes.data(), (const char*)bytes.data() + bytes.size());
#endif
    }

    // Kein BOM: meistens ANSI/UTF-8 ohne BOM.
    // Für FlyFF-Textdaten ist UTF-8 ohne BOM häufig OK. Wenn du „ANSI“ brauchst,
    // wäre der nächste Schritt: optional CP1252/ACP -> UTF-8 konvertieren.
    if (detectedEncoding) *detectedEncoding = "unknown(assume-utf8/ansi)";
    return std::string((const char*)bytes.data(), (const char*)bytes.data() + bytes.size());
}

static void splitLines(const std::string& text, std::vector<std::string>& outLines)
{
    std::string line;
    std::istringstream ss(text);
    while (std::getline(ss, line))
    {
        // CR entfernen
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        outLines.push_back(std::move(line));
    }
}

namespace fs = std::filesystem;
using namespace modules::parser;

data::TokenData UniversalParser::parse(const FileEntry& file, const std::string& resourceRoot)
{
    data::TokenData out;
    out.moduleId   = file.moduleId;
    out.sourceFile = file.filename;

    std::ifstream in(file.absolutePath, std::ios::binary);
    if (!in.is_open())
    {
        core::Log::error("Parser: Failed to open file: " + file.absolutePath);
        return out;
    }

    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

    std::string enc;
    std::string utf8 = bytesToUtf8(bytes, &enc);

    //core::Log::info("Parser: decoding '" + file.filename + "' as " + enc);

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

    // core::Log::info(
    //     "Parsed " + std::to_string(out.tokens.size()) +
    //     " lines from " + file.filename +
    //     " [" + file.moduleId + "]");

    return out;
}
