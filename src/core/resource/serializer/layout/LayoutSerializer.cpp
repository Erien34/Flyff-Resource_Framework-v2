#include "LayoutSerializer.h"
#include "Log.h"
#include "resource/parse/TokenData.h"

#include <cctype>
#include <sstream>

using namespace modules::serializer;
using namespace data::resource::canonical;

static bool isComment(const data::Token& t) { return t.type == "comment"; }

static bool isIdentifier(const data::Token& t)
{
    // wir verlassen uns auf type=="identifier" – wenn dein Tokenizer da sauber ist, perfekt
    // ansonsten fallback: beginnt mit Buchstabe/_.
    if (t.type == "identifier") return true;
    if (t.value.empty()) return false;
    const unsigned char c = (unsigned char)t.value[0];
    return std::isalpha(c) || t.value[0] == '_';
}

static bool isIntegerToken(const data::Token& t)
{
    // Tokenizer kann Zahlen als value/raw liefern – wir prüfen Inhalt.
    if (t.value.empty()) return false;
    size_t i = 0;
    if (t.value[0] == '-' || t.value[0] == '+') i = 1;
    if (i >= t.value.size()) return false;
    for (; i < t.value.size(); ++i)
        if (!std::isdigit((unsigned char)t.value[i])) return false;
    return true;
}

static int toInt(const data::Token& t, int def = 0)
{
    try { return std::stoi(t.value); }
    catch (...) { return def; }
}

static std::string unquote(const std::string& s)
{
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
        return s.substr(1, s.size() - 2);
    return s;
}

// Skip comments + optional symbols that are irrelevant
static void skipIgnorable(const std::vector<data::Token>& toks, size_t& i)
{
    while (i < toks.size())
    {
        if (isComment(toks[i])) { ++i; continue; }
        // häufige „Trenner“ in DSLs: , ; { } ( ) – je nach Parser
        if (toks[i].type == "symbol")
        {
            const auto& v = toks[i].value;
            if (v == "," || v == ";" ) { ++i; continue; }
        }
        break;
    }
}

// Versucht: WTYPE_*  WIDC_*  <visual>  <flags>  x y w h
// <visual> kann identifier oder "quoted string" sein
static bool tryParseControl(const std::vector<data::Token>& toks, size_t& i, RawControl& out)
{
    const size_t start = i;
    skipIgnorable(toks, i);

    if (i >= toks.size()) return false;

    // 1) rawType
    if (!isIdentifier(toks[i])) { i = start; return false; }
    const auto rawTypeTok = toks[i];
    const std::string rawType = rawTypeTok.value;

    // Heuristik: Nur UI Controls, die mit WTYPE_ anfangen (Minimalstand)
    if (rawType.rfind("WTYPE_", 0) != 0)
    {
        i = start;
        return false;
    }
    ++i; skipIgnorable(toks, i);

    // 2) id
    if (i >= toks.size() || !isIdentifier(toks[i])) { i = start; return false; }
    const auto idTok = toks[i];
    const std::string id = idTok.value;
    ++i; skipIgnorable(toks, i);

    // 3) visualRef (identifier oder quoted string)
    if (i >= toks.size()) { i = start; return false; }
    const auto visTok = toks[i];
    std::string vis = visTok.value;
    if (!(isIdentifier(visTok) || (vis.size() >= 2 && vis.front()=='"' && vis.back()=='"')))
    {
        i = start; return false;
    }
    vis = unquote(vis);
    ++i; skipIgnorable(toks, i);

    // 4) flags
    if (i >= toks.size() || !isIntegerToken(toks[i])) { i = start; return false; }
    const auto flagsTok = toks[i];
    const int flags = toInt(flagsTok, 0);
    ++i; skipIgnorable(toks, i);

    // 5-8) x y w h
    if (i + 3 >= toks.size()) { i = start; return false; }
    if (!isIntegerToken(toks[i]) || !isIntegerToken(toks[i+1]) ||
        !isIntegerToken(toks[i+2]) || !isIntegerToken(toks[i+3]))
    {
        i = start; return false;
    }

    const int x = toInt(toks[i], 0);
    const int y = toInt(toks[i+1], 0);
    const int w = toInt(toks[i+2], 0);
    const int h = toInt(toks[i+3], 0);

    // consume them
    const auto provTok = rawTypeTok; // provenance am besten am Starttoken festmachen
    i += 4;

    out = RawControl{};
    out.rawType = rawType;
    out.id = id;
    out.rawVisualRef = vis;
    out.rawFlags = (std::uint32_t)flags;
    out.rect = RawRect{ x, y, w, h };
    out.prov = RawProvenance{
        "",
        static_cast<uint32_t>(provTok.line),
        static_cast<uint32_t>(provTok.column)
    };

    return true;
}

void LayoutSerializer::serialize(const std::vector<data::TokenData>& streams)
{
    m_data = rawLayoutData{};
    m_data.version = 1;

    if (streams.empty())
    {
        m_data.valid = false;
        m_data.warnings.push_back("LayoutSerializer: no streams.");
        core::Log::info("LayoutSerializer: collected 0 streams");
        return;
    }

    m_data.valid = true;
    m_data.streams.reserve(streams.size());

    int totalControls = 0;
    int totalWindows = 0;

    for (size_t s = 0; s < streams.size(); ++s)
    {
        const auto& in = streams[s];

        RawLayoutStream out;
        out.moduleId = in.moduleId;
        out.sourceFile = in.sourceFile;
        out.domain = in.domain;
        out.rawTokens = in.tokens;

        // Minimalstand: 1 Window pro SourceFile/Stream
        RawWindow win;
        win.id = !in.sourceFile.empty() ? in.sourceFile : ("layout_stream_" + std::to_string(s));
        win.prov.sourceFile = in.sourceFile;

        // Tokenliste roh scannen und Controls extrahieren
        const auto& toks = in.tokens;
        size_t i = 0;

        while (i < toks.size())
        {
            RawControl c;
            const size_t before = i;

            if (tryParseControl(toks, i, c))
            {
                c.prov.sourceFile = in.sourceFile;
                win.controls.push_back(std::move(c));
                ++totalControls;
                continue;
            }

            // nichts erkannt -> weiter
            i = before + 1;
        }

        if (win.controls.empty())
        {
            m_data.warnings.push_back("LayoutSerializer: stream '" + win.id + "' produced 0 controls.");
        }
        else
        {
            out.windows.push_back(std::move(win));
            ++totalWindows;
        }

        m_data.streams.push_back(std::move(out));
    }

    publishModel(outputModel(), m_data);

    core::Log::info(
        "LayoutSerializer: collected " + std::to_string(streams.size()) + " streams | " +
        std::to_string(totalWindows) + " windows | " +
        std::to_string(totalControls) + " controls"
    );
}
