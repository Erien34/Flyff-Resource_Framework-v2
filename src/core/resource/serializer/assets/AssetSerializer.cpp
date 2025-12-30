#include "core/resource/serializer/assets/AssetSerializer.h"
#include "TokenData.h"
#include "core/Log.h"

#include <sstream>
#include <iomanip>

using namespace data::module::rawasset;

namespace modules::serializer
{

AssetSerializer::AssetSerializer()
    : SerializerBase()
{
}

void AssetSerializer::serialize(
    const std::vector<data::TokenData>& streams
)
{
    RawAssetData out;

    for (const auto& stream : streams)
    {
        RawAssetFile file;
        file.filename = stream.sourceFile;

        std::vector<RawAssetBlock*> stack;

        // impliziter Root-Block
        file.roots.emplace_back();
        stack.push_back(&file.roots.back());

        for (const auto& token : stream.tokens)
        {
            const std::string& line = token.value;

            if (line == "{")
                continue;

            if (line == "}")
            {
                if (stack.size() > 1)
                    stack.pop_back();
                continue;
            }

            // Block-Label
            if (!line.empty() && line.front() == '"')
            {
                RawAssetBlock blk;
                blk.label = line.substr(1, line.size() - 2);

                stack.back()->children.push_back(std::move(blk));
                stack.push_back(&stack.back()->children.back());
                continue;
            }

            // Entry
            RawAssetEntry entry;
            if (parseEntryLine(line, entry))
            {
                stack.back()->entries.push_back(std::move(entry));
            }
        }

        out.files.push_back(std::move(file));
    }

    // 🔥 DAS IST DER FIX 🔥
    // exakt wie DefineSerializer
    m_data = std::move(out);

    core::Log::info("[AssetSerializer] Asset data serialized");
}

bool AssetSerializer::parseEntryLine(
    const std::string& line,
    RawAssetEntry& out
) const
{
    std::istringstream ss(line);

    if (!(ss >> std::quoted(out.name)))
        return false;

    ss >> out.objectId;
    ss >> out.modelType;
    ss >> std::quoted(out.part);

    ss >> out.pick;
    ss >> out.distance;
    ss >> out.fly;
    ss >> out.scale;
    ss >> out.transparent;
    ss >> out.shadow;
    ss >> out.textureEx;
    ss >> out.value;

    return !ss.fail();
}

} // namespace modules::serializer
