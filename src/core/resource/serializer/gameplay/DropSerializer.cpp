#include "DropSerializer.h"
#include "Log.h"
#include <algorithm>

using namespace modules::serializer;
using namespace data::module::rawdrops;

static bool isValidDropBlockHeader(const std::string& s)
{
    if (s.empty())
        return false;

    // Kommentar-Zeilen ausschließen
    if (s.starts_with("/*") || s.starts_with("//"))
        return false;

    // 1) Reine numerische IDs (z.B. "1", "25")
    bool numeric = std::all_of(
        s.begin(),
        s.end(),
        [](unsigned char c)
        {
            return std::isdigit(c) != 0;
        }
        );
    if (numeric)
        return true;

    // 2) QUEST_*
    if (s.starts_with("QUEST_"))
        return true;

    // 3) EVENT_*
    if (s.starts_with("EVENT_"))
        return true;

    return false;
}

static void logDropSummary(const rawDropData& d,
                           size_t maxDropsPerStream,
                           size_t maxFieldsPerDrop)
{
    core::Log::info(
        "[DropDump] streams=" + std::to_string(d.streams.size())
        );

    for (size_t si = 0; si < d.streams.size(); ++si)
    {
        const auto& s = d.streams[si];

        core::Log::info(
            "  [Stream " + std::to_string(si) + "] file=" +
            s.sourceFile +
            " flatDrops=" + std::to_string(s.flatDrops.size()) +
            " blocks=" + std::to_string(s.blocks.size()) +
            " tokens=" + std::to_string(s.rawTokens.size())
            );

        // ----------------------------
        // Flat drops preview
        // ----------------------------
        size_t shown = 0;
        for (const auto& drop : s.flatDrops)
        {
            if (shown >= maxDropsPerStream)
                break;

            std::string preview;
            size_t fmax = std::min(maxFieldsPerDrop, drop.fields.size());

            for (size_t i = 0; i < fmax; ++i)
            {
                if (i > 0) preview += " | ";
                preview += drop.fields[i];
            }

            if (drop.fields.size() > fmax)
                preview += " | ...";

            core::Log::info(
                "    [Drop " + std::to_string(shown) + "] line=" +
                std::to_string(drop.line) +
                " fields=" + std::to_string(drop.fields.size()) +
                " [" + preview + "]"
                );

            ++shown;
        }

        if (s.flatDrops.size() > maxDropsPerStream)
        {
            core::Log::info(
                "    ... (" +
                std::to_string(s.flatDrops.size() - maxDropsPerStream) +
                " more flat drops)"
                );
        }

        // ----------------------------
        // Block summary
        // ----------------------------
        for (size_t bi = 0; bi < s.blocks.size(); ++bi)
        {
            const auto& b = s.blocks[bi];

            core::Log::info(
                "    [Block " + std::to_string(bi) + "] header=" +
                b.header +
                " bodyEntries=" + std::to_string(b.body.size()) +
                " line=" + std::to_string(b.headerLine)
                );
        }
    }
}

void DropSerializer::serialize(const std::vector<data::TokenData>& streams)
{
    m_data = {};
    m_data.valid = true;

    int totalFlatDrops = 0;
    int totalBlocks    = 0;

    for (const auto& in : streams)
    {
        RawDropStream out{};
        out.moduleId   = in.moduleId;
        out.sourceFile = in.sourceFile;
        out.domain     = in.domain;
        out.rawTokens  = in.tokens;

        bool allowBlocks =
            in.sourceFile == "propGuildQuest.inc";

        bool inBlock = false;
        RawDropBlock currentBlock{};

        for (const auto& tok : in.tokens)
        {
            std::string line = normalizeWhitespace(tok.value);
            if (line.empty())
                continue;

            auto fields = splitByTab(line);
            if (fields.empty())
                continue;

            // -------------------------------------------------
            // FLAT DROP (immer!)
            // -------------------------------------------------
            RawDropEntry entry;
            entry.line   = tok.line;
            entry.fields = fields;

            out.flatDrops.push_back(entry);
            ++totalFlatDrops;

            if (!allowBlocks)
                continue;

            // -------------------------------------------------
            // BLOCK HEADER
            // -------------------------------------------------
            if (!inBlock && fields.size() == 1 &&
                isValidDropBlockHeader(fields[0]))
            {
                inBlock = true;
                currentBlock = {};
                currentBlock.header     = fields[0];
                currentBlock.headerLine = tok.line;
                continue;
            }

            // -------------------------------------------------
            // BLOCK START
            // -------------------------------------------------
            if (inBlock && line == "{")
                continue;

            // -------------------------------------------------
            // BLOCK BODY
            // -------------------------------------------------
            if (inBlock && line != "}")
            {
                currentBlock.body.push_back(entry);
                continue;
            }

            // -------------------------------------------------
            // BLOCK END
            // -------------------------------------------------
            if (inBlock && line == "}")
            {
                out.blocks.push_back(currentBlock);
                ++totalBlocks;
                inBlock = false;
                continue;
            }
        }

        if (inBlock)
        {
            core::Log::warn(
                "[DropSerializer] Block not closed in file " +
                in.sourceFile + " – auto-flush"
                );
            out.blocks.push_back(currentBlock);
            ++totalBlocks;
        }

        m_data.streams.push_back(std::move(out));
    }

    core::Log::info(
        "[DropSerializer] RESULT: " +
        std::to_string(m_data.streams.size()) + " streams, " +
        std::to_string(totalFlatDrops) + " flat drops, " +
        std::to_string(totalBlocks) + " blocks"
        );

    //logDropSummary(m_data, 3, 6);
    publishModel(outputModel(), m_data);
}
