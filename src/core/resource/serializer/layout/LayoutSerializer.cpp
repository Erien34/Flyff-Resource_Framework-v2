#include "LayoutSerializer.h"
#include "Log.h"

#include <sstream>
#include <cctype>
#include <algorithm>

using namespace modules::serializer;
using namespace data::resource::canonical;

// ------------------------------------------------------------
// helpers
// ------------------------------------------------------------
static inline std::string trim(std::string s)
{
    auto notSpace = [](unsigned char c){ return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    return s;
}

static inline bool startsWith(const std::string& s, const char* prefix)
{
    const size_t n = std::char_traits<char>::length(prefix);
    return s.size() >= n && s.compare(0, n, prefix) == 0;
}

static inline bool isIgnorableLine(const std::string& line)
{
    const std::string t = trim(line);
    if (t.empty()) return true;
    if (startsWith(t, "//")) return true;
    if (t == "{" || t == "}") return true;
    return false;
}

static inline std::string unquote(const std::string& s)
{
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
        return s.substr(1, s.size() - 2);
    return s;
}

static std::vector<std::string> splitWs(const std::string& line)
{
    std::istringstream iss(line);
    std::vector<std::string> out;
    std::string tok;
    while (iss >> tok) out.push_back(tok);
    return out;
}

static bool parseHex32(const std::string& in, uint32_t& out)
{
    std::string s = trim(in);
    if (s.empty()) return false;

    // normalize
    for (char& c : s) c = (char)std::toupper((unsigned char)c);
    if (startsWith(s, "0X")) s.erase(0, 2);
    if (!s.empty() && s.back() == 'L') s.pop_back();

    // must be hex digits
    for (char c : s)
    {
        const bool ok = (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F');
        if (!ok) return false;
    }

    try
    {
        unsigned long v = std::stoul(s, nullptr, 16);
        out = (uint32_t)v;
        return true;
    }
    catch (...)
    {
        return false;
    }
}

static bool tryParseInt(const std::string& s, int32_t& out)
{
    try
    {
        size_t idx = 0;
        long v = std::stol(s, &idx, 10);
        if (idx != s.size()) return false;
        out = (int32_t)v;
        return true;
    }
    catch (...) { return false; }
}

static bool lineLooksLikeWindowHeader(const std::string& line)
{
    const std::string t = trim(line);
    return startsWith(t, "APP_");
}

static bool lineLooksLikeControlHeader(const std::string& line)
{
    // robust: manche Zeilen können IDS_ enthalten usw.
    // ControlHeader: beginnt typischerweise mit WTYPE_ und hat WIDC_
    const std::string t = trim(line);
    if (!startsWith(t, "WTYPE_")) return false;
    return t.find("WIDC_") != std::string::npos;
}

static bool lineLooksLikeIds(const std::string& line)
{
    const std::string t = trim(line);
    return startsWith(t, "IDS_");
}

// ------------------------------------------------------------
// parsing: Window header (wie in deinem Editor-Layout)
// Erwartete grobe Struktur (>= 8 tokens):
// 0: APP_*
// 1: "tile.tga"
// 2: "titleOrEmpty" oder IDS_.. oder ""
// 3: mode
// 4: w
// 5: h
// 6: flagsHex
// 7: mod
// ------------------------------------------------------------
static bool parseWindowHeaderLine(
    const std::string& line,
    const uint32_t lineNo,
    const uint32_t colNo,
    RawWindow& outWin)
{
    auto p = splitWs(line);
    if (p.size() < 8) return false;
    if (!startsWith(p[0], "APP_")) return false;

    RawWindow w{};
    w.rawHeaderLine = line;
    w.id = p[0];

    w.tile = unquote(p[1]);
    w.titleTextRaw = p[2]; // roh lassen (kann "" oder IDS_ sein)
    int32_t tmp = 0;

    if (!tryParseInt(p[3], tmp)) return false;
    w.mode = tmp;

    if (!tryParseInt(p[4], tmp)) return false;
    w.w = tmp;

    if (!tryParseInt(p[5], tmp)) return false;
    w.h = tmp;

    w.flagsHex = p[6];
    uint32_t mask = 0;
    if (parseHex32(w.flagsHex, mask))
    {
        // Auto-Fix wie dein Editor: low-flags nach high schieben
        if (mask > 0 && mask < 0x10000)
            mask <<= 16;
        w.flagsMask = mask;
    }
    else
    {
        w.flagsMask = 0;
    }

    if (!tryParseInt(p[7], tmp)) return false;
    w.mod = tmp;

    w.prov = RawProvenance{ "", lineNo, colNo };

    outWin = std::move(w);
    return true;
}

// ------------------------------------------------------------
// parsing: Control header
// Struktur (wie in deinem Editor):
// 0: type
// 1: id
// 2: texture
// 3: mod0
// 4-7: x y x1 y1
// 8: flagsHex
// 9-12: mod1..mod4 (optional)
// 13-15: color (optional) (rgb oder packed in p[13])
// ------------------------------------------------------------
static bool parseControlHeaderLine(
    const std::string& line,
    const uint32_t lineNo,
    const uint32_t colNo,
    RawControl& outCtrl)
{
    auto p = splitWs(line);
    if (p.size() < 9) return false;

    if (!startsWith(p[0], "WTYPE_")) return false;
    if (!startsWith(p[1], "WIDC_")) return false;

    RawControl c{};
    c.rawHeaderLine = line;
    c.rawType = p[0];
    c.id = p[1];
    c.rawVisualRef = unquote(p[2]);

    int32_t tmp = 0;
    if (!tryParseInt(p[3], tmp)) return false;
    c.mod0 = tmp;

    // rect: 4 ints
    if (!tryParseInt(p[4], c.rect.x))  return false;
    if (!tryParseInt(p[5], c.rect.y))  return false;
    if (!tryParseInt(p[6], c.rect.x1)) return false;
    if (!tryParseInt(p[7], c.rect.y1)) return false;

    c.flagsHex = p[8];
    uint32_t mask = 0;
    if (parseHex32(c.flagsHex, mask))
    {
        c.flagsMask = mask;
        c.lowFlags  = (uint16_t)(mask & 0xFFFF);
        c.midFlags  = (uint8_t)((mask >> 16) & 0xFF);
        c.highFlags = (uint8_t)((mask >> 24) & 0xFF);
    }
    else
    {
        c.flagsMask = 0;
    }

    // mods 1..4
    if (p.size() >= 13)
    {
        bool ok = true;
        ok &= tryParseInt(p[9],  c.mod1);
        ok &= tryParseInt(p[10], c.mod2);
        ok &= tryParseInt(p[11], c.mod3);
        ok &= tryParseInt(p[12], c.mod4);
        c.hasMods1to4 = ok;
        if (!ok)
        {
            c.mod1 = c.mod2 = c.mod3 = c.mod4 = 0;
            c.hasMods1to4 = false;
        }
    }

    // color: entweder RGB (3 ints 0..255) oder packed in p[13]
    c.color = RawColor{};
    if (p.size() >= 16)
    {
        int32_t r=0,g=0,b=0;
        const bool okR = tryParseInt(p[13], r);
        const bool okG = tryParseInt(p[14], g);
        const bool okB = tryParseInt(p[15], b);

        if (okR && okG && okB &&
            r >= 0 && r <= 255 &&
            g >= 0 && g <= 255 &&
            b >= 0 && b <= 255)
        {
            c.color.hasRgb = true;
            c.color.r = (uint8_t)r;
            c.color.g = (uint8_t)g;
            c.color.b = (uint8_t)b;
            c.color.mode = "rgb";
        }
        else
        {
            // packed fallback
            int32_t packed = 0;
            if (tryParseInt(p[13], packed))
            {
                c.color.hasPacked = true;
                c.color.packed = (uint32_t)packed;
                c.color.r = (uint8_t)((c.color.packed >> 16) & 0xFF);
                c.color.g = (uint8_t)((c.color.packed >> 8)  & 0xFF);
                c.color.b = (uint8_t)((c.color.packed)       & 0xFF);
                c.color.hasRgb = true;
                c.color.mode = "packed";
            }
            else
            {
                c.color.mode = "default";
            }
        }
    }
    else
    {
        c.color.mode = "default";
    }

    c.prov = RawProvenance{ "", lineNo, colNo };

    outCtrl = std::move(c);
    return true;
}

// ------------------------------------------------------------
// Logging (kompakt, damit Log nicht explodiert)
// ------------------------------------------------------------
static void logLayoutSummary(const rawLayoutData& d, int maxWindowsPerStream = 3, int maxControlsPerWindow = 5)
{
    using core::Log;

    Log::info("[LayoutDump] streams=" + std::to_string(d.streams.size()));

    for (size_t si = 0; si < d.streams.size(); ++si)
    {
        const auto& s = d.streams[si];
        Log::info(
            "  [Stream " + std::to_string(si) + "] file=" + s.sourceFile +
            " domain=" + s.domain +
            " windows=" + std::to_string(s.windows.size()) +
            " tokens=" + std::to_string(s.rawTokens.size())
            );

        const size_t wcount = std::min<size_t>(s.windows.size(), (size_t)std::max(0, maxWindowsPerStream));
        for (size_t wi = 0; wi < wcount; ++wi)
        {
            const auto& w = s.windows[wi];
            Log::info(
                "    [Window " + std::to_string(wi) + "] id=" + w.id +
                " tile=" + w.tile +
                " size=" + std::to_string(w.w) + "x" + std::to_string(w.h) +
                " controls=" + std::to_string(w.controls.size())
                );

            const size_t ccount = std::min<size_t>(w.controls.size(), (size_t)std::max(0, maxControlsPerWindow));
            for (size_t ci = 0; ci < ccount; ++ci)
            {
                const auto& c = w.controls[ci];
                Log::info(
                    "      [Control] id=" + c.id +
                    " type=" + c.rawType +
                    " tex=" + c.rawVisualRef +
                    " mod0=" + std::to_string(c.mod0) +
                    " rect=(" + std::to_string(c.rect.x) + "," + std::to_string(c.rect.y) + "," +
                    std::to_string(c.rect.x1) + "," + std::to_string(c.rect.y1) + ")" +
                    " flags=" + c.flagsHex
                    );
            }
        }

        if (s.windows.size() > wcount)
            Log::info("    ... (" + std::to_string(s.windows.size() - wcount) + " more windows)");
    }
}

// ------------------------------------------------------------
// Serializer
// ------------------------------------------------------------
void LayoutSerializer::serialize(const std::vector<data::TokenData>& streams)
{
    m_data = rawLayoutData{};
    m_data.schema = RawLayoutVersion{1, 0};

    if (streams.empty())
    {
        m_data.valid = false;
        m_data.warnings.push_back("LayoutSerializer: no streams.");
        core::Log::info("[LayoutSerializer] RESULT: 0 streams");
        publishModel(outputModel(), m_data);
        return;
    }

    m_data.valid = true;
    m_data.streams.reserve(streams.size());

    int totalWindows = 0;
    int totalControls = 0;

    for (const auto& in : streams)
    {
        RawLayoutStream out{};
        out.moduleId = in.moduleId;
        out.sourceFile = in.sourceFile;
        out.domain = in.domain;
        out.rawTokens = in.tokens; // Snapshot

        RawWindow currentWin{};
        bool hasWindow = false;

        // state: sammeln von Window-IDs (IDS_...) vor Controls
        bool beforeFirstControlInWindow = false;
        int windowTextCount = 0;

        // state: für Control Title/Tooltip
        RawControl* lastControl = nullptr;
        int controlTextCount = 0;

        auto flushWindow = [&]()
        {
            if (!hasWindow) return;

            if (currentWin.prov.sourceFile.empty())
                currentWin.prov.sourceFile = in.sourceFile;
            for (auto& c : currentWin.controls)
            {
                if (c.prov.sourceFile.empty())
                    c.prov.sourceFile = in.sourceFile;
            }

            out.windows.push_back(std::move(currentWin));
            currentWin = RawWindow{};
            hasWindow = false;

            beforeFirstControlInWindow = false;
            windowTextCount = 0;
            lastControl = nullptr;
            controlTextCount = 0;

            ++totalWindows;
        };

        // Tokens durchgehen: UniversalParser liefert in Token.value i.d.R. ganze Zeilen
        for (size_t i = 0; i < in.tokens.size(); ++i)
        {
            const auto& tok = in.tokens[i];
            const std::string line = tok.value;

            if (isIgnorableLine(line))
                continue;

            // Window start?
            if (lineLooksLikeWindowHeader(line))
            {
                RawWindow w;
                if (parseWindowHeaderLine(line, (uint32_t)tok.line, (uint32_t)tok.column, w))
                {
                    // altes Fenster beenden
                    flushWindow();

                    // neues Fenster beginnen
                    w.prov.sourceFile = in.sourceFile;
                    currentWin = std::move(w);
                    hasWindow = true;

                    beforeFirstControlInWindow = true;
                    windowTextCount = 0;
                    lastControl = nullptr;
                    controlTextCount = 0;
                    continue;
                }
            }

            // Control?
            if (lineLooksLikeControlHeader(line))
            {
                RawControl c;
                if (parseControlHeaderLine(line, (uint32_t)tok.line, (uint32_t)tok.column, c))
                {
                    if (!hasWindow)
                    {
                        // fallback: falls Controls ohne APP_ vorhanden
                        currentWin = RawWindow{};
                        currentWin.id = in.sourceFile.empty() ? "window_unknown" : in.sourceFile;
                        currentWin.prov = RawProvenance{ in.sourceFile, (uint32_t)tok.line, (uint32_t)tok.column };
                        hasWindow = true;
                    }

                    c.prov.sourceFile = in.sourceFile;
                    currentWin.controls.push_back(std::move(c));
                    ++totalControls;

                    // nach erstem Control: Window-Textphase endet
                    beforeFirstControlInWindow = false;

                    // control text phase beginnt
                    lastControl = &currentWin.controls.back();
                    controlTextCount = 0;
                    continue;
                }
            }

            // IDS_ lines -> zuordnen:
            if (lineLooksLikeIds(line))
            {
                const std::string ids = trim(line);

                // 1) Window-IDs direkt nach Header, vor Controls: Title + Help
                if (hasWindow && beforeFirstControlInWindow && windowTextCount < 2)
                {
                    if (windowTextCount == 0) currentWin.windowTitleId = ids;
                    else if (windowTextCount == 1) currentWin.windowHelpId = ids;
                    ++windowTextCount;
                    continue;
                }

                // 2) Control-IDs nach ControlHeader: Title + Tooltip
                if (lastControl && controlTextCount < 2)
                {
                    if (controlTextCount == 0) lastControl->titleId = ids;
                    else if (controlTextCount == 1) lastControl->tooltipId = ids;
                    ++controlTextCount;
                    continue;
                }

                // extra IDS_* ignorieren wir erstmal (aber bleiben im rawTokens Snapshot erhalten)
                continue;
            }

            // sonstige Zeilen: ignorieren (Snapshot bleibt erhalten)
        }

        // letztes Fenster flushen
        flushWindow();

        if (out.windows.empty())
            m_data.warnings.push_back("LayoutSerializer: stream '" + out.sourceFile + "' produced 0 windows.");

        m_data.streams.push_back(std::move(out));
    }

    publishModel(outputModel(), m_data);

    core::Log::info(
        "[LayoutSerializer] RESULT: " +
        std::to_string(m_data.streams.size()) + " streams, " +
        std::to_string(totalWindows) + " windows, " +
        std::to_string(totalControls) + " controls"
        );

    if (!m_data.warnings.empty())
        core::Log::warn("[LayoutSerializer] WARNINGS: " + std::to_string(m_data.warnings.size()));

    // kompakter dump (sonst explodiert dein Log bei 471 windows / 3.5k controls)
    logLayoutSummary(m_data, /*maxWindowsPerStream*/3, /*maxControlsPerWindow*/5);
}
