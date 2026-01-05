#include "JobSerializer.h"
#include "Log.h"

using namespace modules::serializer;
using namespace data::module::rawjobs;

void JobSerializer::serialize(const std::vector<data::TokenData>& streams)
{
    m_data = {};
    m_data.valid = true;

    int totalJobs = 0;

    // -------------------------------------------------
    // Streams = Dateien
    // -------------------------------------------------
    for (const auto& in : streams)
    {
        RawJobStream out{};
        out.moduleId   = in.moduleId;
        out.sourceFile = in.sourceFile;
        out.domain     = in.domain;
        out.rawTokens  = in.tokens; // Snapshot

        // -------------------------------------------------
        // Zeilen → Jobs
        // -------------------------------------------------
        for (const auto& tok : in.tokens)
        {
            std::string line = normalizeWhitespace(tok.value);
            if (line.empty())
                continue;

            auto fields = splitByTab(line);
            if (fields.empty())
                continue;

            RawJobEntry e;
            e.line   = tok.line;
            e.fields = std::move(fields);

            out.jobs.push_back(std::move(e));
            ++totalJobs;
        }

        m_data.streams.push_back(std::move(out));
    }

    // -------------------------------------------------
    // RESULT-Log (Item/Layout-Style)
    // -------------------------------------------------
    core::Log::info(
        "[JobSerializer] RESULT: " +
        std::to_string(m_data.streams.size()) + " streams, " +
        std::to_string(totalJobs) + " jobs"
        );

    // kompakter Dump (max 3 Jobs pro Datei)
    auto logJobSummary = [](const rawJobData& d,
                            size_t maxJobsPerStream,
                            size_t maxFieldsPerJob)
    {
        core::Log::info("[JobDump] streams=" + std::to_string(d.streams.size()));

        for (size_t si = 0; si < d.streams.size(); ++si)
        {
            const auto& s = d.streams[si];
            core::Log::info(
                "  [Stream " + std::to_string(si) + "] file=" +
                s.sourceFile + " jobs=" + std::to_string(s.jobs.size()) +
                " tokens=" + std::to_string(s.rawTokens.size())
                );

            size_t shown = 0;
            for (const auto& j : s.jobs)
            {
                if (shown >= maxJobsPerStream)
                    break;

                std::string preview;
                size_t fmax = std::min(maxFieldsPerJob, j.fields.size());
                for (size_t i = 0; i < fmax; ++i)
                {
                    if (i > 0) preview += " | ";
                    preview += j.fields[i];
                }
                if (j.fields.size() > fmax)
                    preview += " | ...";

                core::Log::info(
                    "    [Job " + std::to_string(shown) + "] line=" +
                    std::to_string(j.line) +
                    " fields=" + std::to_string(j.fields.size()) +
                    " [" + preview + "]"
                    );

                ++shown;
            }

            if (s.jobs.size() > maxJobsPerStream)
                core::Log::info(
                    "    ... (" +
                    std::to_string(s.jobs.size() - maxJobsPerStream) +
                    " more jobs)"
                    );
        }
    };

    //logJobSummary(m_data, 3, 6);

    publishModel(outputModel(), m_data);
}
