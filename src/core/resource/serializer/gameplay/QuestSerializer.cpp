#include "QuestSerializer.h"
#include "Log.h"

using namespace modules::serializer;
using namespace data::module::rawquest;

// --------------------------------------------
// Hilfsfunktionen
// --------------------------------------------
static bool isValidQuestBlockHeader(const std::string& s)
{
    if (s.empty())
        return false;

    // Kommentare ausschließen
    if (s.starts_with("/*"))
        return false;

    // QUEST_*
    if (s.starts_with("QUEST_"))
        return true;

    // EVENT_*
    if (s.starts_with("EVENT_"))
        return true;

    // Numerisch (z.B. State 0)
    bool numeric = true;
    for (char c : s)
    {
        if (!std::isdigit(static_cast<unsigned char>(c)))
        {
            numeric = false;
            break;
        }
    }
    return numeric;
}

// --------------------------------------------
// Serializer
// --------------------------------------------
void QuestSerializer::serialize(const std::vector<data::TokenData>& streams)
{
    m_data = {};
    m_data.valid = true;

    int totalFlat = 0;
    int totalBlocks = 0;

    // ==========================================
    // Streams = Dateien
    // ==========================================
    for (const auto& in : streams)
    {
        RawQuestStream out{};
        out.moduleId   = in.moduleId;
        out.sourceFile = in.sourceFile;
        out.domain     = in.domain;
        out.rawTokens  = in.tokens;

        bool inBlock = false;
        RawQuestBlock currentBlock{};

        // --------------------------------------
        // Zeilen
        // --------------------------------------
        for (const auto& tok : in.tokens)
        {
            std::string line = normalizeWhitespace(tok.value);
            if (line.empty())
                continue;

            auto fields = splitByTab(line);
            if (fields.empty())
                continue;

            // ----------------------------------
            // Flat Entry (immer!)
            // ----------------------------------
            RawQuestEntry entry;
            entry.line   = tok.line;
            entry.fields = fields;

            out.flatEntries.push_back(entry);
            ++totalFlat;

            // ----------------------------------
            // Block Header
            // ----------------------------------
            if (!inBlock &&
                fields.size() == 1 &&
                isValidQuestBlockHeader(fields[0]))
            {
                inBlock = true;
                currentBlock = {};
                currentBlock.header     = fields[0];
                currentBlock.headerLine = tok.line;
                continue;
            }

            // ----------------------------------
            // Block Start
            // ----------------------------------
            if (inBlock && line == "{")
                continue;

            // ----------------------------------
            // Block Body
            // ----------------------------------
            if (inBlock && line != "}")
            {
                currentBlock.body.push_back(entry);
                continue;
            }

            // ----------------------------------
            // Block End
            // ----------------------------------
            if (inBlock && line == "}")
            {
                out.blocks.push_back(currentBlock);
                ++totalBlocks;
                inBlock = false;
                continue;
            }
        }

        // --------------------------------------
        // Auto-Flush
        // --------------------------------------
        if (inBlock)
        {
            core::Log::warn(
                "[QuestSerializer] Block not closed in file " +
                in.sourceFile + " – auto-flush"
                );
            out.blocks.push_back(currentBlock);
            ++totalBlocks;
        }

        m_data.streams.push_back(std::move(out));
    }

    // ==========================================
    // RESULT
    // ==========================================
    core::Log::info(
        "[QuestSerializer] RESULT: " +
        std::to_string(m_data.streams.size()) + " streams, " +
        std::to_string(totalFlat) + " flat entries, " +
        std::to_string(totalBlocks) + " blocks"
        );

    // ==========================================
    // Dump (kompakt)
    // ==========================================
    auto logQuestSummary =
        [](const rawQuestData& d,
           size_t maxBlocks,
           size_t maxEntries)
    {
        core::Log::info("[QuestDump] streams=" + std::to_string(d.streams.size()));

        for (size_t si = 0; si < d.streams.size(); ++si)
        {
            const auto& s = d.streams[si];
            core::Log::info(
                "  [Stream " + std::to_string(si) +
                "] file=" + s.sourceFile +
                " flat=" + std::to_string(s.flatEntries.size()) +
                " blocks=" + std::to_string(s.blocks.size())
                );

            size_t shown = 0;
            for (const auto& b : s.blocks)
            {
                if (shown >= maxBlocks)
                    break;

                core::Log::info(
                    "    [Block " + std::to_string(shown) +
                    "] header=" + b.header +
                    " bodyEntries=" + std::to_string(b.body.size()) +
                    " line=" + std::to_string(b.headerLine)
                    );

                ++shown;
            }

            if (s.blocks.size() > maxBlocks)
            {
                core::Log::info(
                    "    ... (" +
                    std::to_string(s.blocks.size() - maxBlocks) +
                    " more blocks)"
                    );
            }
        }
    };

    //logQuestSummary(m_data, 3, 6);
    publishModel(outputModel(), m_data);
}
