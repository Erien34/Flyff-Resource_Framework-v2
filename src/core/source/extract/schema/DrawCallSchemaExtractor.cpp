#include "DrawCallSchemaExtractor.h"
#include "core/source/extract/util/TextUnit.h"
#include <regex>

namespace core::source::extract::rules
{
    static bool relContains(const std::filesystem::path& p, const char* token)
    {
        return p.generic_string().find(token) != std::string::npos;
    }

    void DrawCallSchemaExtractor::extract(
        const std::vector<data::source::SourceGroup>& groups,
        std::vector<ExtractFact>& outFacts
    ) const
    {
        using namespace core::source::extract::util;

        // Patterns (v1)
        std::regex reDIP(R"(DrawIndexedPrimitive\s*\()");
        std::regex reTriList(R"(D3DPT_TRIANGLELIST)");
        std::regex reSetGroup(R"(\bSetGroup\s*\()");
        std::regex reSetTextureEx(R"(\bSetTextureEx\s*\()");

        for (const auto& g : groups)
        {
            for (const auto& f : g.files)
            {
                const auto rel = f.relativePath;

                const bool isObject3Dcpp   = relContains(rel, "_Common/Object3D.cpp");
                const bool isModelObjectcpp= relContains(rel, "_Common/ModelObject.cpp");
                const bool isObjcpp        = relContains(rel, "_Common/Obj.cpp");

                if (!isObject3Dcpp && !isModelObjectcpp && !isObjcpp)
                    continue;

                auto unit = TextUnit::load(f.absolutePath, rel);

                auto emitHit = [&](const char* factType, std::size_t off, float conf, int ctx=2)
                {
                    int line = unit.lineOfOffset(off);
                    auto snip = unit.snippetAtLine(line, ctx);

                    ExtractFact fact;
                    fact.factType = factType;
                    fact.extractorId = id();
                    fact.extractorVersion = version();
                    fact.file = rel.generic_string();
                    fact.line = line;
                    fact.confidence = conf;
                    fact.payload =
                        std::string("{\"groupId\":\"") + jsonEscape(g.id) +
                        "\",\"snippet\":\"" + jsonEscape(snip) + "\"}";
                    outFacts.emplace_back(std::move(fact));
                };

                // (A) Object3D.cpp: DrawIndexedPrimitive signals => per-submesh drawcall
                if (isObject3Dcpp)
                {
                    for (auto it = std::sregex_iterator(unit.content.begin(), unit.content.end(), reDIP);
                         it != std::sregex_iterator(); ++it)
                    {
                        emitHit("DrawCall.Invoke.DrawIndexedPrimitive", (std::size_t)it->position(), 1.0f);
                        emitHit("DrawCall.PerSubMesh", (std::size_t)it->position(), 0.9f);
                    }

                    for (auto it = std::sregex_iterator(unit.content.begin(), unit.content.end(), reTriList);
                         it != std::sregex_iterator(); ++it)
                    {
                        emitHit("DrawCall.PrimitiveType.TriangleList", (std::size_t)it->position(), 1.0f, 1);
                    }

                    // Mapping hint: block fields used as DIP args (v1: keyword based)
                    std::regex reMapping(R"(\b(m_nStartVertex|m_nPrimitiveCount|m_nTextureID)\b)");
                    for (auto it = std::sregex_iterator(unit.content.begin(), unit.content.end(), reMapping);
                         it != std::sregex_iterator(); ++it)
                    {
                        emitHit("DrawCall.Mapping.MaterialBlockField", (std::size_t)it->position(), 0.8f, 1);
                    }
                }

                // (B) Obj.cpp: LOD grouping and TextureEx (affects which drawcalls happen / material variant)
                if (isObjcpp)
                {
                    for (auto it = std::sregex_iterator(unit.content.begin(), unit.content.end(), reSetGroup);
                         it != std::sregex_iterator(); ++it)
                    {
                        emitHit("DrawCall.Grouping.SetGroup", (std::size_t)it->position(), 0.95f, 2);
                    }

                    for (auto it = std::sregex_iterator(unit.content.begin(), unit.content.end(), reSetTextureEx);
                         it != std::sregex_iterator(); ++it)
                    {
                        emitHit("Material.Variant.SetTextureEx", (std::size_t)it->position(), 0.9f, 2);
                    }
                }

                // (C) ModelObject.cpp: bridge call chain (CModelObject -> Object3D render)
                if (isModelObjectcpp)
                {
                    std::regex reRenderCall(R"(\bRender\s*\(\s*pd3dDevice)");
                    for (auto it = std::sregex_iterator(unit.content.begin(), unit.content.end(), reRenderCall);
                         it != std::sregex_iterator(); ++it)
                    {
                        emitHit("DrawCall.Delegation.ModelObjectRender", (std::size_t)it->position(), 0.7f, 2);
                    }
                }
            }
        }
    }
}
