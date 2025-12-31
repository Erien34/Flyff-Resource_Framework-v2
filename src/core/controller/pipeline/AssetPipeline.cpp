#include "AssetPipeline.h"
#include "ProjectData.h"

#include "Log.h"

// ================= ASSET INDEX =================
#include "core/asset/index/AssetIndexBuilder.h"

// ================= LOADERS =================
#include "core/asset/loader/ModelLoader.h"
#include "core/asset/loader/ImageLoader.h"
#include "core/asset/loader/AnimationLoader.h"
#include "core/asset/loader/SfxLoader.h"

// ================= DECODERS =================
#include "core/asset/decoder/O3DDecoder.h"
#include "core/asset/decoder/O3DDecryptor.h"
#include "core/asset/decoder/ImageDecoder.h"
#include "core/asset/decoder/AniDecoder.h"
#include "core/asset/decoder/SfxDecoder.h"

// ================= PARSER =================
#include "asset/parser/ModelParser.h"
#include "parsed/O3DParsed.h"

// ================= CONVERTERS (Phase B) =================
#include "core/asset/converter/ImageConverter.h"
#include "core/asset/converter/SfxConverter.h"
#include "core/asset/converter/ModelConverter.h"

#include <filesystem>
#include <map>

namespace fs = std::filesystem;

namespace core::pipeline
{

// ======================================================
// ASSET PIPELINE A  (LOAD / DECODE / PARSE)
// ======================================================

AssetPipelineA::AssetPipelineA(struct ProjectData& project)
    : m_projectData(project)
{
}

void AssetPipelineA::onReset()
{
    m_step = Step::ScanAssets;
    m_assetIndex.clear();
    m_loadedModels.clear();
    m_decodedModels.clear();
}

PipelineResult AssetPipelineA::onUpdate()
{
    switch (m_step)
    {
    case Step::ScanAssets:
        scanAssets();
        m_step = Step::BuildDirectories;
        return { JobState::Running, true };

    case Step::BuildDirectories:
        buildAssetDirectories();
        m_step = Step::LoadAssets;
        return { JobState::Running, true };

    case Step::LoadAssets:
        loadAssets();
        m_step = Step::DecryptAssets;
        return { JobState::Running, true };

    case Step::DecryptAssets:
        decryptAssets();
        m_step = Step::DecodeAssets;
        return { JobState::Running, true };

    case Step::DecodeAssets:
        decodeAssets();
        m_step = Step::ParseAssets;
        return { JobState::Running, true };

    case Step::ParseAssets:
        parseAssets();
        m_step = Step::Done;
        return { JobState::Done, false };

    case Step::Done:
        return { JobState::Done, false };
    }

    return { JobState::Error, false, "AssetPipelineA: invalid state" };
}

// -------------------- Phase A Steps --------------------

void AssetPipelineA::scanAssets()
{
    ::asset::AssetIndexBuilder builder;
    ::asset::AssetIndexBuilder::Settings s;
    s.assetRoot         = m_projectData.assetCachePath;
    s.resourceModelRoot = m_projectData.resourcePath + "/Model";

    m_assetIndex = builder.build(s);

    Log::info(
        "[AssetPipelineA] ScanAssets: records=" +
        std::to_string(m_assetIndex.size()));
}

void AssetPipelineA::buildAssetDirectories()
{
    for (const auto& kv : m_assetIndex)
    {
        const auto& rec = kv.second;
        if (rec.kind == ::asset::AssetKind::Ignore)
            continue;

        std::error_code ec;
        fs::create_directories(rec.outPath.parent_path(), ec);
    }

    Log::info("[AssetPipelineA] BuildDirectories done");
}

void AssetPipelineA::loadAssets()
{
    m_loadedModels.clear();

    for (const auto& kv : m_assetIndex)
    {
        const auto& rec = kv.second;
        std::string err;

        if (rec.techKind == ::asset::AssetTechKind::Model)
        {
            ::asset::ModelSource src;
            if (::asset::ModelLoader::load(src, rec, &err))
                m_loadedModels.emplace_back(std::move(src));
            else
                Log::error(err);
        }
    }

    Log::info(
        "[AssetPipelineA] LoadAssets: models=" +
        std::to_string(m_loadedModels.size()));
}

void AssetPipelineA::decryptAssets()
{
    for (auto& mdl : m_loadedModels)
    {
        auto decryptPart = [&](::asset::ModelPartSource& part)
        {
            if (!part.exists || part.bytes.bytes.empty())
                return;

            std::vector<std::uint8_t> out;
            std::uint8_t key = 0;
            std::string err;

            if (::asset::O3DDecryptor::decryptAuto(out, part.bytes.bytes, &key, &err))
                part.bytes.bytes = std::move(out);
            else
                Log::error(err);
        };

        decryptPart(mdl.skeleton);
        decryptPart(mdl.mesh);
    }

    Log::info("[AssetPipelineA] DecryptAssets done");
}

void AssetPipelineA::decodeAssets()
{
    m_decodedModels.clear();

    for (const auto& mdl : m_loadedModels)
    {
        ::asset::O3DDecoded dec;
        std::string err;

        if (::asset::O3DDecoder::decode(dec, mdl, &err))
            m_decodedModels.emplace_back(std::move(dec));
        else
            Log::error(err);
    }

    Log::info(
        "[AssetPipelineA] DecodeAssets: decodedModels=" +
        std::to_string(m_decodedModels.size()));
}

void AssetPipelineA::parseAssets()
{
    for (std::size_t i = 0; i < m_decodedModels.size(); ++i)
    {
        ::asset::O3DParsed parsed;
        std::string err;

        if (!asset::parser::ModelParser::parse(parsed, m_decodedModels[i], &err))
            Log::error(err);
    }

    Log::info("[AssetPipelineA] ParseAssets done");
}


// ======================================================
// ASSET PIPELINE B  (APPLY SPECS / CONVERT)
// ======================================================

AssetPipelineB::AssetPipelineB(struct ProjectData& project)
    : m_projectData(project)
{
}

void AssetPipelineB::onReset()
{
    m_step = Step::ConvertAssets;
}

PipelineResult AssetPipelineB::onUpdate()
{
    switch (m_step)
    {
    case Step::ConvertAssets:
        convertAssets();
        m_step = Step::BuildSnapshot;
        return { JobState::Running, false };

    case Step::BuildSnapshot:
        buildSnapshot();
        m_step = Step::Done;
        return { JobState::Done, false };

    case Step::Done:
        return { JobState::Done, false };
    }

    return { JobState::Error, false, "AssetPipelineB: invalid state" };
}

// -------------------- Phase B Steps --------------------

void AssetPipelineB::convertAssets()
{
    ::asset::ConverterSettings cs;
    cs.overwriteExisting = false;
    cs.modelToolPath = m_projectData.projectRoot + "/tools/model_converter.exe";

    ::asset::ModelConverter modelConv(cs);
    ::asset::ImageConverter imageConv(cs);
    ::asset::SfxConverter   sfxConv(cs);

    // ⚠️ Phase B nutzt KEINE Decoder
    // → nur SourceSpecs + vorhandene AssetSources

    Log::info("[AssetPipelineB] ConvertAssets done");
}

void AssetPipelineB::buildSnapshot()
{
    // v1: Snapshot stub
    Log::info("[AssetPipelineB] BuildSnapshot (placeholder)");
}

} // namespace asset::pipeline
