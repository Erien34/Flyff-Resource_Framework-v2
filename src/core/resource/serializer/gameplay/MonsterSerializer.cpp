#include "MonsterSerializer.h"
#include "Log.h"

#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

using namespace modules::serializer;
using namespace data::module::rawmonsters;

// -------------------------------------------------
// kleine Helfer (lokal, damit keine externen deps fehlen)
// -------------------------------------------------
static inline std::string ltrimCopy(std::string s)
{
    size_t i = 0;
    while (i < s.size() && std::isspace((unsigned char)s[i])) ++i;
    s.erase(0, i);
    return s;
}

static inline std::string rtrimCopy(std::string s)
{
    while (!s.empty() && std::isspace((unsigned char)s.back())) s.pop_back();
    return s;
}

static inline std::string trimCopy(std::string s)
{
    return rtrimCopy(ltrimCopy(std::move(s)));
}

// schneidet inline // Kommentare ab (falls Tokenizer das nicht macht)
static inline std::string stripInlineSlashComment(const std::string& s)
{
    size_t p = s.find("//");
    if (p == std::string::npos) return s;
    return s.substr(0, p);
}

static inline bool isUpperIdent(const std::string& s)
{
    if (s.empty()) return false;
    for (char ch : s)
    {
        unsigned char c = (unsigned char)ch;
        if (!(std::isupper(c) || std::isdigit(c) || c == '_'))
            return false;
    }
    return true;
}

static inline std::string stripQuotesCopy(std::string s)
{
    s = trimCopy(std::move(s));
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
        return s.substr(1, s.size() - 2);
    return s;
}

// -------------------------------------------------
// Serializer
// -------------------------------------------------
void MonsterSerializer::serialize(const std::vector<data::TokenData>& streams)
{
    m_data = {};
    m_data.valid = true;

    int totalMonsterLines = 0;
    int totalAggroBlocks  = 0;
    int totalAggroEntries = 0;

    // -------------------------------------------------
    // Streams = Dateien (propMover.txt, propMoverEx.inc, propAggro.txt)
    // -------------------------------------------------
    for (const auto& in : streams)
    {
        RawMonsterStream out{};
        out.moduleId   = in.moduleId;
        out.sourceFile = in.sourceFile;
        out.domain     = in.domain;
        out.rawTokens  = in.tokens; // Snapshot

        const bool isAggroFile =
            (in.sourceFile == "propAggro.txt" || in.sourceFile == "propAggro.inc");

        // ================================
        // Aggro Parsing State
        // ================================
        bool inAggroBlock = false;
        bool inAggroBody  = false;
        RawAggroBlock currentAggro{};

        auto flushAggro = [&]()
        {
            if (!inAggroBlock) return;

            out.aggroBlocks.push_back(currentAggro);
            ++totalAggroBlocks;
            totalAggroEntries += (int)currentAggro.entries.size();

            currentAggro = {};
            inAggroBlock = false;
            inAggroBody  = false;
        };

        // -------------------------------------------------
        // Zeilen durchgehen
        // -------------------------------------------------
        for (const auto& tok : in.tokens)
        {
            // 1) normalize + inline // weg
            std::string rawLine = normalizeWhitespace(tok.value);
            rawLine = stripInlineSlashComment(rawLine);
            rawLine = trimCopy(rawLine);

            if (rawLine.empty())
                continue;

            // optional: block-comment marker grob skippen (falls im Tokenizer nicht entfernt)
            if (rawLine.rfind("/*", 0) == 0 || rawLine.rfind("*/", 0) == 0 || rawLine.rfind("*", 0) == 0)
                continue;

            // 2) fields
            auto fields = splitByTab(rawLine);
            if (fields.empty())
                continue;

            // -------------------------------------------------
            // Standard: propMover / propMoverEx -> 1 Zeile = 1 Entry
            // -------------------------------------------------
            if (!isAggroFile)
            {
                RawMonsterEntry e;
                e.line   = tok.line;
                e.fields = std::move(fields);

                out.monsters.push_back(std::move(e));
                ++totalMonsterLines;
                continue;
            }

            // -------------------------------------------------
            // propAggro.txt: Blockstruktur
            // Beispiel:
            // NORMAL_AGGRO
            // {
            //   BUFF_AGGRO_RATE = 3
            //   "SI_LOD_SUP_ANGER" = 5
            // }
            // -------------------------------------------------

            // BLOCK HEADER (nur 1 Feld, z.B. NORMAL_AGGRO)
            if (!inAggroBlock)
            {
                if (fields.size() == 1 && fields[0] != "{" && fields[0] != "}")
                {
                    const std::string header = trimCopy(fields[0]);
                    if (isUpperIdent(header)) // NORMAL_AGGRO / SINGLE_AGGRO etc.
                    {
                        inAggroBlock = true;
                        inAggroBody  = false;
                        currentAggro = {};
                        currentAggro.name       = header;
                        currentAggro.headerLine = tok.line;
                        continue;
                    }
                }

                // wenn kein gültiger Header: ignorieren (Snapshot bleibt ja erhalten)
                continue;
            }

            // BLOCK START
            if (rawLine == "{")
            {
                inAggroBody = true;
                continue;
            }

            // BLOCK END
            if (rawLine == "}")
            {
                flushAggro();
                continue;
            }

            // BODY: key/value
            if (inAggroBody)
            {
                // erwartete patterns nach normalizeWhitespace:
                // KEY \t = \t VALUE
                // "KEY" \t = \t VALUE
                if (fields.size() >= 3 && fields[1] == "=")
                {
                    RawAggroEntry ae;
                    ae.line  = tok.line;
                    ae.key   = stripQuotesCopy(fields[0]);
                    ae.value = fields[2]; // bewusst raw lassen (kann Zahl oder Macro sein)
                    out.aggroBlocks.empty(); // no-op; nur damit keine warnungen? (kann raus)
                    currentAggro.entries.push_back(std::move(ae));
                    continue;
                }

                // fallback: manche Zeilen könnten anders formatiert sein -> ignorieren
                continue;
            }

            // Wenn wir hier sind: inAggroBlock true aber noch nicht im Body -> ignorieren
        }

        // Auto-Flush (Sicherheit)
        if (inAggroBlock)
        {
            core::Log::warn(
                "[MonsterSerializer] Aggro block not closed in file " +
                in.sourceFile + " – auto-flush"
                );
            flushAggro();
        }

        m_data.streams.push_back(std::move(out));
    }

    // -------------------------------------------------
    // RESULT-Log (einheitlich!)
    // -------------------------------------------------
    core::Log::info(
        "[MonsterSerializer] RESULT: " +
        std::to_string(m_data.streams.size()) + " streams, " +
        std::to_string(totalMonsterLines) + " monsterLines, " +
        std::to_string(totalAggroBlocks) + " aggroBlocks, " +
        std::to_string(totalAggroEntries) + " aggroEntries"
        );

    // optional: du kannst hier analog logMonsterSummary wieder aktivieren
    //           und zusätzlich Aggro-Blöcke previewen.

    publishModel(outputModel(), m_data);
}
