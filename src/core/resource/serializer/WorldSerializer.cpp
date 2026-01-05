#include "WorldSerializer.h"
#include "Log.h"
#include "resource/parse/TokenData.h"

#include <unordered_set>
#include <filesystem>
#include <sstream>

using namespace modules::serializer;
using namespace data::module::rawworld;

void WorldSerializer::serialize(const std::vector<data::TokenData>& streams)
{
    m_data.areas.clear();

    // ----------------------------------------
    // Phase 1: Deduplizierung
    // ----------------------------------------
    std::unordered_set<std::string> seenFiles;
    std::vector<const data::TokenData*> unique;

    for (const auto& s : streams)
    {
        if (s.domain != "world")
            continue;

        if (seenFiles.insert(s.absolutePath).second)
            unique.push_back(&s);
    }

    // ----------------------------------------
    // Zähler (LOKAL!)
    // ----------------------------------------
    size_t streamCount     = unique.size();
    size_t settingsCount   = 0;
    size_t regionCount     = 0;
    size_t localizedCount  = 0;

    // ----------------------------------------
    // Phase 2+3: Parsing
    // ----------------------------------------
    for (const auto* stream : unique)
    {
        const std::string& file = stream->sourceFile;
        const std::string areaKey =
            std::filesystem::path(file).stem().string();

        auto& area = m_data.areas[areaKey];
        area.key = areaKey;

        // -----------------------------
        // .wld → World Settings
        // -----------------------------
        if (file.ends_with(".wld"))
        {
            for (const auto& t : stream->tokens)
            {
                std::istringstream ss(t.value);
                std::string key;
                ss >> key;

                if (key == "indoor")      ss >> area.settings.indoor;
                else if (key == "fly")   ss >> area.settings.fly;
                else if (key == "ambient") ss >> std::hex >> area.settings.ambient;
                else if (key == "bgColor") ss >> std::hex >> area.settings.bgColor;
                else if (key == "camera")
                {
                    ss >> area.settings.camPos[0]
                        >> area.settings.camPos[1]
                        >> area.settings.camPos[2]
                        >> area.settings.camRot[0]
                        >> area.settings.camRot[1]
                        >> area.settings.camRot[2];
                }
            }
            ++settingsCount;
        }

        // -----------------------------
        // .rgn → Regions
        // -----------------------------
        else if (file.ends_with(".rgn"))
        {
            for (const auto& t : stream->tokens)
            {
                if (!t.value.starts_with("region"))
                    continue;

                Region r{};
                std::istringstream ss(t.value);
                ss >> r.type >> r.id
                    >> r.x1 >> r.y1 >> r.x2 >> r.y2;

                area.regions.push_back(std::move(r));
                ++regionCount;
            }
        }

        // -----------------------------
        // .txt → Localization
        // -----------------------------
        else if (file.ends_with(".txt"))
        {
            for (const auto& t : stream->tokens)
            {
                auto tab = t.value.find('\t');
                if (tab == std::string::npos)
                    continue;

                area.title = t.value.substr(tab + 1);
                ++localizedCount;
                break;
            }
        }
    }

    // ----------------------------------------
    // Finaler PRODUKTIONS-LOG
    // ----------------------------------------
    core::Log::info(
        "[WorldSerializer] RESULT: " +
        std::to_string(streamCount)   + " streams, " +
        std::to_string(settingsCount) + " settings, " +
        std::to_string(regionCount)   + " regions, " +
        std::to_string(localizedCount)+ " localized"
        );
}
