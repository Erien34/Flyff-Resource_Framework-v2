#include "SubMeshSchemaExtractor.h"
#include "core/source/extract/util/TextUnit.h"
#include <regex>

namespace core::source::extract::rules
{
    static bool fileEndsWith(const std::filesystem::path& p, const char* suffix)
    {
        auto s = p.generic_string();
        if (s.size() < std::strlen(suffix)) return false;
        return s.compare(s.size() - std::strlen(suffix), std::strlen(suffix), suffix) == 0;
    }

    static bool groupLikelyCommonObject3D(const data::source::SourceGroup& g)
    {
        // v1: robust: any file path contains "_Common/Object3D"
        for (const auto& f : g.files)
        {
            auto rp = f.relativePath.generic_string();
            if (rp.find("_Common/Object3D") != std::string::npos) return true;
        }
        return false;
    }

    void SubMeshSchemaExtractor::extract(
        const std::vector<data::source::SourceGroup>& groups,
        std::vector<ExtractFact>& outFacts
    ) const
    {
        using namespace core::source::extract::util;

        for (const auto& g : groups)
        {
            if (!groupLikelyCommonObject3D(g))
                continue;

            for (const auto& f : g.files)
            {
                const auto rel = f.relativePath;
                if (!fileEndsWith(rel, "Object3D.h") && !fileEndsWith(rel, "Object3D.cpp"))
                    continue;

                auto unit = TextUnit::load(f.absolutePath, rel);

                // (A) struct MATERIAL_BLOCK fields (header)
                if (fileEndsWith(rel, "Object3D.h"))
                {
                    // Capture body of struct MATERIAL_BLOCK {...};
                    std::regex re(R"(struct\s+MATERIAL_BLOCK\s*\{([\s\S]*?)\};)");
                    std::smatch m;
                    if (std::regex_search(unit.content, m, re))
                    {
                        std::string body = m[1].str();
                        // field lines: very simple v1 - find "<type> <name>;" and collect names
                        std::regex fieldRe(R"(([A-Za-z_]\w*(?:\s*::\s*[A-Za-z_]\w*)?(?:\s*[\*\&])?)\s+([A-Za-z_]\w*)\s*(?:\[[^\]]+\])?\s*;)");
                        std::sregex_iterator it(body.begin(), body.end(), fieldRe), end;

                        std::string fieldsJson = "[";
                        bool first = true;
                        for (; it != end; ++it)
                        {
                            std::string name = (*it)[2].str();
                            if (!first) fieldsJson += ",";
                            first = false;
                            fieldsJson += "\"" + jsonEscape(name) + "\"";
                        }
                        fieldsJson += "]";

                        ExtractFact fact;
                        fact.factType = "SubMesh.Block.Struct";
                        fact.extractorId = id();
                        fact.extractorVersion = version();
                        fact.file = rel.generic_string();
                        fact.line = 1;
                        fact.confidence = 1.0f;
                        fact.payload =
                            std::string("{\"struct\":\"MATERIAL_BLOCK\",\"fields\":") + fieldsJson + "}";
                        outFacts.emplace_back(std::move(fact));
                    }
                }

                // (B) file->Read of MATERIAL_BLOCK array (cpp)
                if (fileEndsWith(rel, "Object3D.cpp"))
                {
                    // Look for Read(... sizeof(MATERIAL_BLOCK) * ... )
                    std::regex reRead(R"(Read\s*\([^;]*sizeof\s*\(\s*MATERIAL_BLOCK\s*\)[^;]*\)\s*;)");
                    auto begin = std::sregex_iterator(unit.content.begin(), unit.content.end(), reRead);
                    auto end = std::sregex_iterator();
                    for (auto it = begin; it != end; ++it)
                    {
                        std::size_t off = (std::size_t)it->position();
                        int line = unit.lineOfOffset(off);

                        ExtractFact fact;
                        fact.factType = "SubMesh.Block.ArrayRead";
                        fact.extractorId = id();
                        fact.extractorVersion = version();
                        fact.file = rel.generic_string();
                        fact.line = line;
                        fact.confidence = 0.95f;

                        auto snip = unit.snippetAtLine(line, 2);
                        fact.payload =
                            std::string("{\"hint\":\"Read MATERIAL_BLOCK array\",\"snippet\":\"") +
                            jsonEscape(snip) + "\"}";
                        outFacts.emplace_back(std::move(fact));
                    }
                }
            }
        }
    }
}
