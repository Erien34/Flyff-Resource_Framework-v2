#include "ItemSerializer.h"
#include "Log.h"

using namespace data::module::rawitems;
#include "ItemSerializer.h"
#include "Log.h"

#include <cctype>
#include <string>
#include <vector>

using namespace modules::serializer;
using namespace data::module::rawitems;

// -------------------------------------------------
// Helpers (lokal, damit keine externen Abhängigkeiten)
// -------------------------------------------------
static std::string trimLocal(const std::string& s)
{
    size_t a = 0;
    while (a < s.size() && std::isspace((unsigned char)s[a])) ++a;
    size_t b = s.size();
    while (b > a && std::isspace((unsigned char)s[b - 1])) --b;
    return s.substr(a, b - a);
}

static bool isNumber(const std::string& s)
{
    if (s.empty()) return false;
    for (char c : s)
        if (!std::isdigit((unsigned char)c))
            return false;
    return true;
}

static std::string stripQuotes(const std::string& s)
{
    std::string t = trimLocal(s);
    if (t.size() >= 2 && (t.front() == '"' && t.back() == '"'))
        return t.substr(1, t.size() - 2);
    return t;
}

static std::string stripParensAndPunct(const std::string& s)
{
    // entfernt () ; , )
    std::string out;
    out.reserve(s.size());
    for (char c : s)
    {
        if (c == '(' || c == ')' || c == ';' || c == ',' )
            continue;
        out.push_back(c);
    }
    return trimLocal(out);
}

static std::string normalizeTypeNodeKey(const std::string& s)
{
    // "title(" -> "title", "parent(" -> "parent"
    std::string t = trimLocal(s);
    while (!t.empty() && (t.back() == '(' || t.back() == ')' || t.back() == ';'))
        t.pop_back();
    return t;
}

static bool looksLikeTypeNodeHeader(const std::vector<std::string>& fields)
{
    // TypeNode startet als "1" alleine (nach normalizeWhitespace -> meist fields.size()==1)
    return (fields.size() == 1 && isNumber(fields[0]));
}

static int findFirstIntInFields(const std::vector<std::string>& fields, int fallback = -1)
{
    for (const auto& f : fields)
    {
        std::string t = stripParensAndPunct(f);
        if (isNumber(t))
            return std::stoi(t);
    }
    return fallback;
}

static std::string findFirstIdsInFields(const std::vector<std::string>& fields)
{
    // findet IDS_* auch wenn zerlegt: IDS_TEXTCLIENT... oder "IDS_TEXTCLIENT..."
    for (const auto& f : fields)
    {
        std::string t = stripQuotes(stripParensAndPunct(f));
        if (t.rfind("IDS_", 0) == 0)
            return t;
        if (t.find("IDS_") != std::string::npos)
        {
            // falls vorne/innen noch Müll hängt
            auto pos = t.find("IDS_");
            return t.substr(pos);
        }
    }
    return {};
}

static std::string findFirstTypeToken(const std::vector<std::string>& fields, const char* prefix)
{
    for (const auto& f : fields)
    {
        std::string t = stripQuotes(stripParensAndPunct(f));
        if (t.rfind(prefix, 0) == 0)
            return t;
        if (t.find(prefix) != std::string::npos)
        {
            auto pos = t.find(prefix);
            return t.substr(pos);
        }
    }
    return {};
}

// -------------------------------------------------
// Optional: sehr kompakter Summary-Dump (falls du ihn willst)
// (Wenn du schon logItemSummary woanders hast, kannst du das entfernen.)
// -------------------------------------------------
static void logItemSummary(const rawItemData& d, int maxItemsPerStream, int maxFieldsPerItem)
{
    core::Log::info("[ItemDump] streams=" + std::to_string(d.streams.size()));

    for (size_t si = 0; si < d.streams.size(); ++si)
    {
        const auto& s = d.streams[si];
        core::Log::info(
            "  [Stream " + std::to_string(si) + "] file=" + s.sourceFile +
            " items=" + std::to_string(s.items.size()) +
            " tokens=" + std::to_string(s.rawTokens.size())
            );

        int shown = 0;
        for (const auto& it : s.items)
        {
            if (shown >= maxItemsPerStream) break;

            std::string preview;
            int cap = (int)it.fields.size();
            if (cap > maxFieldsPerItem) cap = maxFieldsPerItem;

            for (int i = 0; i < cap; ++i)
            {
                if (i) preview += " | ";
                preview += it.fields[i];
            }
            if ((int)it.fields.size() > cap) preview += " | ...";

            core::Log::info(
                "    [Item " + std::to_string(shown) + "] line=" + std::to_string(it.line) +
                " fields=" + std::to_string(it.fields.size()) +
                " [" + preview + "]"
                );
            ++shown;
        }

        if ((int)s.items.size() > maxItemsPerStream)
        {
            core::Log::info("    ... (" + std::to_string((int)s.items.size() - maxItemsPerStream) + " more items)");
        }
    }
}

// -------------------------------------------------
// Serializer
// -------------------------------------------------
void ItemSerializer::serialize(const std::vector<data::TokenData>& streams)
{
    m_data = rawItemData{};
    m_data.valid = true;

    int totalItems = 0;

    // =================================================
    // Streams = Dateien (Layout-analog)
    // =================================================
    for (const auto& in : streams)
    {
        // -------- states (PRO STREAM!) --------
        bool inPiercing = false;
        std::string piercingTarget;

        bool inSetItem = false;
        bool inSetElem = false;
        bool inSetAvail = false;
        bool inInnerBlock = false;
        RawSetItem currentSet;

        bool inRandomOptItem = false;
        bool inRandomOptBlock = false;
        int randomOptBraceDepth = 0;
        RawRandomOptItem currentRandomOpt;

        // TypeNode state
        bool inTypeNode = false;
        bool typeNodeExpectBrace = false; // wichtig: "1" muss wirklich gefolgt sein von "{"
        RawItemTypeNode currentTypeNode;

        // -------- stream output --------
        RawItemStream out{};
        out.moduleId   = in.moduleId;
        out.sourceFile = in.sourceFile;
        out.domain     = in.domain;
        out.rawTokens  = in.tokens;

        // -------------------------------------------------
        // Zeilen
        // -------------------------------------------------
        for (const auto& tok : in.tokens)
        {
            std::string line = normalizeWhitespace(tok.value);
            if (line.empty())
                continue;

            auto fields = splitByTab(line);
            if (fields.empty())
                continue;

            // =============================================
            // GENERISCHES ITEM (1 Zeile = 1 Record)
            // =============================================
            {
                RawItem item;
                item.line   = tok.line;
                item.fields = fields;
                out.items.push_back(std::move(item));
                ++totalItems;
            }

            // =============================================
            // Piercing
            // =============================================
            if (!inPiercing && fields.size() >= 2 && fields[0] == "Piercing")
            {
                inPiercing = true;
                piercingTarget = fields[1];
                continue;
            }

            if (inPiercing && fields.size() >= 2 &&
                fields[0] != "{" && fields[0] != "}")
            {
                RawModifier mod;
                mod.type   = "Piercing";
                mod.target = piercingTarget;
                mod.key    = fields[0];
                mod.value  = fields[1];
                m_data.modifiers.push_back(std::move(mod));
                continue;
            }

            if (inPiercing && line == "}")
            {
                inPiercing = false;
                piercingTarget.clear();
                continue;
            }

            // =============================================
            // RandomOptItem
            // =============================================
            if (!inRandomOptItem && fields.size() >= 5 && fields[0] == "RandomOptItem")
            {
                inRandomOptItem = true;
                randomOptBraceDepth = 0;

                currentRandomOpt = {};
                currentRandomOpt.id        = fields[1];
                currentRandomOpt.nameToken = fields[2];
                currentRandomOpt.paramA    = fields[3];
                currentRandomOpt.paramB    = fields[4];
                continue;
            }

            // Öffnende Klammer
            if (inRandomOptItem && line == "{")
            {
                ++randomOptBraceDepth;
                continue;
            }

            // Effekt-Zeilen (nur INNEN)
            if (inRandomOptItem && randomOptBraceDepth > 0 &&
                fields.size() >= 2 && line != "{")
            {
                RawRandomOptEffect eff;
                eff.stat  = fields[0];
                eff.value = fields[1];
                currentRandomOpt.effects.push_back(eff);
                continue;
            }

            // Schließende Klammer
            if (inRandomOptItem && line == "}")
            {
                --randomOptBraceDepth;

                // Block vollständig beendet
                if (randomOptBraceDepth == 0)
                {
                    m_data.randomOptItems.push_back(currentRandomOpt);
                    inRandomOptItem = false;
                }
                continue;
            }

            // =============================================
            // SetItem
            // =============================================
            if (!inSetItem && fields.size() >= 3 && fields[0] == "SetItem")
            {
                inSetItem = true;
                currentSet = {};
                currentSet.setId     = fields[1];
                currentSet.nameToken = fields[2];
                continue;
            }

            if (inSetItem && fields[0] == "Elem")
            {
                inSetElem = true;
                inSetAvail = false;
                continue;
            }

            if (inSetItem && fields[0] == "Avail")
            {
                inSetAvail = true;
                inSetElem = false;
                continue;
            }

            if ((inSetElem || inSetAvail) && line == "{")
            {
                inInnerBlock = true;
                continue;
            }

            if (inSetElem && inInnerBlock && fields.size() >= 2)
            {
                currentSet.elements.push_back({ fields[0], fields[1] });
                continue;
            }

            if (inSetAvail && inInnerBlock && fields.size() >= 3)
            {
                currentSet.effects.push_back({ fields[0], fields[1], fields[2] });
                continue;
            }

            if (line == "}")
            {
                if (inInnerBlock)
                {
                    inInnerBlock = false;
                    continue;
                }

                if (inSetItem)
                {
                    m_data.setItems.push_back(currentSet);
                    inSetItem = inSetElem = inSetAvail = false;
                    continue;
                }
            }

            // =============================================
            // Item Type / Slot Hierarchie (TypeNodes)
            // =============================================
            // Start: "1" (allein) -> aber nur gültig wenn danach wirklich "{"
            if (!inTypeNode && looksLikeTypeNodeHeader(fields))
            {
                inTypeNode = true;
                typeNodeExpectBrace = true;
                currentTypeNode = {};
                currentTypeNode.id = std::stoi(fields[0]);
                continue;
            }

            if (inTypeNode && typeNodeExpectBrace)
            {
                // nächste relevante Zeile muss "{"
                if (line == "{")
                {
                    typeNodeExpectBrace = false;
                    continue;
                }

                // war doch kein TypeNode -> abbrechen (keine Daten verlieren)
                inTypeNode = false;
                typeNodeExpectBrace = false;
                continue;
            }

            if (inTypeNode)
            {
                if (line == "}")
                {
                    m_data.itemTypes.push_back(currentTypeNode);
                    inTypeNode = false;
                    typeNodeExpectBrace = false;
                    continue;
                }

                // parse lines wie: title( "IDS_..." );  oder parent( 1 );  oder type1( TYPE1_... );
                std::string key = normalizeTypeNodeKey(fields[0]);

                if (key == "parent")
                {
                    int pid = findFirstIntInFields(fields, -1);
                    if (pid >= 0) currentTypeNode.parentId = pid;
                    continue;
                }
                if (key == "title")
                {
                    std::string ids = findFirstIdsInFields(fields);
                    if (!ids.empty()) currentTypeNode.titleId = ids;
                    continue;
                }
                if (key == "type1")
                {
                    std::string t = findFirstTypeToken(fields, "TYPE1_");
                    if (!t.empty()) currentTypeNode.type1 = t;
                    continue;
                }
                if (key == "type2")
                {
                    std::string t = findFirstTypeToken(fields, "TYPE2_");
                    if (!t.empty()) currentTypeNode.type2 = t;
                    continue;
                }

                // unbekannt -> ignorieren (rawTokens + items behalten alles)
                continue;
            }
        }

        // Auto-flush Safety (falls RandomOptItem offen bleibt)
        if (inRandomOptItem)
        {
            core::Log::warn(
                "[ItemSerializer] RandomOptItem not closed in file " +
                in.sourceFile + " – auto-flush"
                );
            m_data.randomOptItems.push_back(currentRandomOpt);
        }

        // Auto-flush Safety (falls TypeNode offen bleibt)
        if (inTypeNode && !typeNodeExpectBrace)
        {
            core::Log::warn(
                "[ItemSerializer] TypeNode not closed in file " +
                in.sourceFile + " – auto-flush"
                );
            m_data.itemTypes.push_back(currentTypeNode);
        }

        m_data.streams.push_back(std::move(out));
    }

    // =================================================
    // RESULT
    // =================================================
    core::Log::info(
        "[ItemSerializer] RESULT: " +
        std::to_string(m_data.streams.size()) + " streams, " +
        std::to_string(totalItems) + " items, " +
        std::to_string(m_data.modifiers.size()) + " modifiers, " +
        std::to_string(m_data.setItems.size()) + " setItems, " +
        std::to_string(m_data.randomOptItems.size()) + " randomOptItems, " +
        std::to_string(m_data.itemTypes.size()) + " itemTypes"
        );

    // optional: analog zum LayoutDump
    //logItemSummary(m_data, 3, 6);

    publishModel(outputModel(), m_data);
}
