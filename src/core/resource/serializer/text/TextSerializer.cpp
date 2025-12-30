#include "TextSerializer.h"
#include "Log.h"

using namespace modules::serializer;
using namespace data::module::rawtext;

void TextSerializer::serialize(const std::vector<data::TokenData>& streams)
{
    m_data.files.clear();
    m_data.valid = false;

    for (const auto& stream : streams)
    {
        const std::string& domain = stream.domain;
        const std::string& file   = stream.sourceFile;

        // core::Log::info(
        //     "[TextSerializer] START file='" + file +
        //     "' domain='" + domain +
        //     "' tokens=" + std::to_string(stream.tokens.size())
        //     );

        auto& fileData = m_data.files[domain][file];

        size_t accepted = 0;
        size_t skipped  = 0;

        for (const auto& token : stream.tokens)
        {
            const std::string& line = token.value;

            if (line.empty())
            {
                ++skipped;
                continue;
            }

            std::string key;
            std::string text;

            // 1) Primär: Tab-Split
            size_t tabPos = line.find('\t');
            if (tabPos != std::string::npos)
            {
                key  = line.substr(0, tabPos);
                text = line.substr(tabPos + 1);

                // Mehrere Tabs → loggen
                if (line.find('\t', tabPos + 1) != std::string::npos)
                {
                    // core::Log::warn(
                    //     "[TextSerializer] Multiple tabs detected in '" +
                    //     file + "' line " + std::to_string(token.line)
                    //     );
                }
            }
            else
            {
                // 2) Fallback: Whitespace-Split
                size_t ws = line.find_first_of(" \t");
                if (ws != std::string::npos)
                {
                    key  = line.substr(0, ws);
                    text = line.substr(ws + 1);
                }
                else
                {
                    // 3) Nur Key, bewusst leerer Text
                    key  = line;
                    text = "";
                }
            }

            if (key.empty())
            {
                ++skipped;
                continue;
            }

            fileData.entries[key] = text;
            ++accepted;
        }

        // core::Log::info(
        //     "[TextSerializer] file='" + file +
        //     "' accepted=" + std::to_string(accepted) +
        //     " skipped=" + std::to_string(skipped)
        //     );
    }

    m_data.valid = !m_data.files.empty();

    // core::Log::info(
    //     "TextSerializer: collected RAW text data for " +
    //     std::to_string(m_data.files.size()) +
    //     " domains"
    //     );
}
