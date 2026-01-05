#include "SkillSerializer.h"
#include "Log.h"

using namespace modules::serializer;
using namespace data::module::rawskills;

void SkillSerializer::serialize(const std::vector<data::TokenData>& streams)
{
    m_data = {};
    m_data.valid = true;

    int totalSkills = 0;

    // -------------------------------------------------
    // Streams = Dateien (propSkill.txt, propTroupeSkill.txt, propSkillAdd.csv)
    // -------------------------------------------------
    for (const auto& in : streams)
    {
        RawSkillStream out{};
        out.moduleId   = in.moduleId;
        out.sourceFile = in.sourceFile;
        out.domain     = in.domain;
        out.rawTokens  = in.tokens; // Snapshot

        // -------------------------------------------------
        // Zeilen → Skill-Einträge
        // -------------------------------------------------
        for (const auto& tok : in.tokens)
        {
            std::string line = normalizeWhitespace(tok.value);
            if (line.empty())
                continue;

            auto fields = splitByTab(line);
            if (fields.empty())
                continue;

            RawSkillEntry e;
            e.line   = tok.line;
            e.fields = std::move(fields);

            out.skills.push_back(std::move(e));
            ++totalSkills;
        }

        m_data.streams.push_back(std::move(out));
    }

    // -------------------------------------------------
    // RESULT-Log (Layout / Item / Job Style)
    // -------------------------------------------------
    core::Log::info(
        "[SkillSerializer] RESULT: " +
        std::to_string(m_data.streams.size()) + " streams, " +
        std::to_string(totalSkills) + " skills"
        );

    // -------------------------------------------------
    // Kompakter Dump (max 3 Skills pro Datei)
    // -------------------------------------------------
    auto logSkillSummary = [](const rawSkillData& d,
                              size_t maxSkillsPerStream,
                              size_t maxFieldsPerSkill)
    {
        core::Log::info("[SkillDump] streams=" + std::to_string(d.streams.size()));

        for (size_t si = 0; si < d.streams.size(); ++si)
        {
            const auto& s = d.streams[si];
            core::Log::info(
                "  [Stream " + std::to_string(si) + "] file=" +
                s.sourceFile + " skills=" + std::to_string(s.skills.size()) +
                " tokens=" + std::to_string(s.rawTokens.size())
                );

            size_t shown = 0;
            for (const auto& sk : s.skills)
            {
                if (shown >= maxSkillsPerStream)
                    break;

                std::string preview;
                size_t fmax = std::min(maxFieldsPerSkill, sk.fields.size());
                for (size_t i = 0; i < fmax; ++i)
                {
                    if (i > 0) preview += " | ";
                    preview += sk.fields[i];
                }
                if (sk.fields.size() > fmax)
                    preview += " | ...";

                core::Log::info(
                    "    [Skill " + std::to_string(shown) + "] line=" +
                    std::to_string(sk.line) +
                    " fields=" + std::to_string(sk.fields.size()) +
                    " [" + preview + "]"
                    );

                ++shown;
            }

            if (s.skills.size() > maxSkillsPerStream)
                core::Log::info(
                    "    ... (" +
                    std::to_string(s.skills.size() - maxSkillsPerStream) +
                    " more skills)"
                    );
        }
    };

    //logSkillSummary(m_data, 3, 6);

    publishModel(outputModel(), m_data);
}
