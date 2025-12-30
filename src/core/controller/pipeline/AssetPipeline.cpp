#include "AssetPipeline.h"

#include "Log.h"

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

// ================= CONVERTERS =================
#include "core/asset/converter/ImageConverter.h"
#include "core/asset/converter/SfxConverter.h"
#include "core/asset/converter/ModelConverter.h"

// ================= PARSER =================
#include "asset/parser/ModelParser.h"
#include "parsed/O3DParsed.h"

#include <filesystem>
#include <map>

using namespace core;
namespace fs = std::filesystem;

AssetPipeline::AssetPipeline(ProjectData& projectData)
    : m_projectData(projectData)
{
}

void AssetPipeline::onReset()
{
    m_state.step = State::Step::ScanAssets;
    m_assetIndexBuilt = false;
}

PipelineResult AssetPipeline::onUpdate()
{
    switch (m_state.step)
    {
    case State::Step::ScanAssets:
        scanAssets();
        m_state.step = State::Step::BuildDirectories;
        return { JobState::Running, true };

    case State::Step::BuildDirectories:
        buildAssetDirectories();
        m_state.step = State::Step::LoadAssets;
        return { JobState::Running, true };

    case State::Step::LoadAssets:
        loadAssets();
        m_state.step = State::Step::DecryptAssets;
        return { JobState::Running, true };

    case State::Step::DecryptAssets:
        decryptAssets();
        m_state.step = State::Step::DecodeAssets;
        return { JobState::Running, true };

    case State::Step::DecodeAssets:
        decodeAssets();
        m_state.step = State::Step::ParseAssets;
        return { JobState::Running, true };

    case State::Step::ParseAssets:
        parseAssets();
        m_state.step = State::Step::ConvertAssets;
        return { JobState::Running, true };

    case State::Step::ConvertAssets:
        //convertAssets();
        m_state.step = State::Step::BuildSnapshot;
        return { JobState::Running, true };

    case State::Step::BuildSnapshot:
        m_state.step = State::Step::Done;
        m_jobState = JobState::Done;
        return { JobState::Done, false };

    case State::Step::Done:
        return { JobState::Done, false };
    }

    m_jobState = JobState::Error;
    m_error = "AssetPipeline: invalid state";
    return { JobState::Error, false, m_error };
}



void AssetPipeline::scanAssets()
{
    if (m_assetIndexBuilt)
        return;

    resource::AssetIndexBuilder builder;
    resource::AssetIndexBuilder::Settings s;
    //s.clientRoot        = m_projectData.clientPath;
    s.assetRoot         = m_projectData.assetCachePath;
    s.resourceModelRoot = m_projectData.resourcePath + "/Model";

    // Log::info(
    //     "[AssetPipeline] ScanAssets: clientRoot='" + s.clientRoot.string() +
    //     "', resourceModelRoot='" + s.resourceModelRoot.string() +
    //     "', assetRoot='" + s.assetRoot.string() + "'"
    //     );

    // 🔹 build index
    m_assetIndex = builder.build(s);
    m_assetIndexBuilt = true;

    // =====================================================
    // SUMMARY
    // =====================================================
    size_t total = m_assetIndex.size();
    size_t withClient = 0;
    size_t withRes = 0;

    std::map<resource::AssetKind, size_t> byKind;

    for (const auto& kv : m_assetIndex)
    {
        const auto& r = kv.second;
        byKind[r.kind]++;

        if (r.existsInClient)   ++withClient;
        if (r.existsInResource) ++withRes;
    }

    Log::info(
        "[AssetPipeline] ScanAssets: records=" + std::to_string(total) +
        " clientPresent=" + std::to_string(withClient) +
        " resourcePresent=" + std::to_string(withRes)
        );

    // =====================================================
    // AssetKind → String (semantisch!)
    // =====================================================
    auto kindToStr = [](resource::AssetKind k) -> std::string
    {
        using resource::AssetKind;
        switch (k)
        {
        case AssetKind::Texture:    return "Texture";
        case AssetKind::Image:      return "Image";
        case AssetKind::UiSprite:   return "UiSprite";
        case AssetKind::Icon:       return "Icon";
        case AssetKind::Model:      return "Model";
        case AssetKind::Animation:  return "Animation";
        case AssetKind::Sound:      return "Sound";
        case AssetKind::Sfx:        return "Sfx";
        case AssetKind::RuntimeChr: return "Chr";
        case AssetKind::Ignore:     return "Ignore";
        default:                    return "Unknown";
        }
    };

    // for (const auto& kv : byKind)
    // {
    //     Log::info(
    //         "[AssetPipeline] ScanAssets: kind=" +
    //         kindToStr(kv.first) +
    //         " count=" + std::to_string(kv.second)
    //         );
    // }

    //Log::info("[AssetPipeline] ScanAssets: END");
}

void AssetPipeline::buildAssetDirectories()
{
    //Log::info("[AssetPipeline] AssetDirectories BEGIN");

    if (!m_assetIndexBuilt || m_assetIndex.empty())
    {
        Log::warn("[AssetPipeline] AssetDirectories: index not built or empty");
        return;
    }

    size_t created = 0;
    size_t failed  = 0;

    for (const auto& kv : m_assetIndex)
    {
        const auto& rec = kv.second;

        // Safety: Ignore darf nie Ordner erzeugen
        if (rec.kind == resource::AssetKind::Ignore)
            continue;

        const fs::path dir = rec.outPath.parent_path();
        if (dir.empty())
            continue;

        std::error_code ec;
        fs::create_directories(dir, ec); // idempotent

        if (ec)
        {
            ++failed;
            Log::error(
                "[AssetPipeline] AssetDirectories: create_directories failed: " +
                dir.string() + " (" + ec.message() + ")"
                );
        }
        else
        {
            // create_directories liefert leider kein "created yes/no" ohne Rückgabewert hier,
            // deshalb zählen wir "attempts" als created. Optional könnten wir vorher exists() prüfen.
            ++created;
        }
    }

    Log::info(
        "[AssetPipeline] AssetDirectories DONE attempts=" + std::to_string(created) +
        " failed=" + std::to_string(failed)
        );
}

void AssetPipeline::loadAssets()
{
    //Log::info("[AssetPipeline] LoadAssets BEGIN");

    m_loadedModels.clear();
    m_loadedImages.clear();
    m_loadedAnimations.clear();
    m_loadedSfx.clear();

    for (const auto& kv : m_assetIndex)
    {
        const resource::AssetRecord& rec = kv.second;
        std::string err;

        switch (rec.techKind)
        {
        case resource::AssetTechKind::Model:
        {
            resource::ModelSource src;
            if (resource::ModelLoader::load(src, rec, &err))
                m_loadedModels.emplace_back(std::move(src));
            else
                Log::error(err);
            break;
        }

            // case resource::AssetTechKind::Texture:
            // case resource::AssetTechKind::Image:
            // {
            //     resource::ImageSource src;
            //     if (resource::ImageLoader::load(src, rec, &err))
            //         m_loadedImages.emplace_back(std::move(src));
            //     else
            //         Log::error(err);
            //     break;
            // }

            // case resource::AssetTechKind::Animation:
            // {
            //     resource::AnimationSource src;
            //     if (resource::AnimationLoader::load(src, rec, &err))
            //         m_loadedAnimations.emplace_back(std::move(src));
            //     else
            //         Log::error(err);
            //     break;
            // }

            // case resource::AssetTechKind::Sfx:
            // {
            //     resource::SfxSource src;
            //     if (resource::SfxLoader::load(src, rec, &err))
            //         m_loadedSfx.emplace_back(std::move(src));
            //     else
            //         Log::error(err);
            //     break;
            // }

        default:
            break;
        }
    }

    //Log::info("[AssetPipeline] LoadAssets DONE");
}

void AssetPipeline::decryptAssets()
{
    //Log::info("[AssetPipeline] DecryptAssets BEGIN");

    // Wir mutieren die geladenen Sources (Memory-only), damit alle nachfolgenden Steps
    // nur noch decrypted Bytes sehen.
    for (auto& src : m_loadedModels)
    {
        std::string err;

        auto decryptPart = [&](resource::ModelPartSource& part, const char* tag)
        {
            if (!part.exists)
                return;

            const auto& in = part.bytes.bytes;
            if (in.empty())
                return;

            std::vector<std::uint8_t> out;
            std::uint8_t usedKey = 0;

            if (!resource::O3DDecryptor::decryptAuto(out, in, &usedKey, &err))
            {
                Log::error(std::string("[AssetDecrypt] ") + tag + " decrypt failed: " + err);
                return;
            }

            // overwriting the bytes in-place
            part.bytes.bytes = std::move(out);

            if (usedKey == 0xFF)
                Log::info(std::string("[AssetDecrypt] ") + tag + " decrypted using key=" + std::to_string((int)usedKey));
        };

        decryptPart(src.skeleton, "O3D skeleton");
        decryptPart(src.mesh,     "O3D mesh");
    }

    //Log::info("[AssetPipeline] DecryptAssets DONE");
}

void AssetPipeline::decodeAssets()
{
    //Log::info("[AssetPipeline] DecodeAssets BEGIN");

    m_decodedModels.clear();
    m_decodedImages.clear();
    m_decodedAni.clear();
    m_decodedSfx.clear();

    // ---- Models (O3D) ----
    for (const auto& src : m_loadedModels)
    {
        resource::O3DDecoded dec;
        std::string err;
        if (resource::O3DDecoder::decode(dec, src, &err))
            m_decodedModels.emplace_back(std::move(dec));
        else
            Log::error(err);
    }

    // ---- Images / DDS ----
    for (const auto& src : m_loadedImages)
    {
        resource::DecodedImageData dec;
        std::string err;

        if (resource::ImageDecoder::decode(dec, src.bytes, &err))
        {
            if (!dec.rgba.empty())   // nur echte Decodes behalten
                m_decodedImages.emplace_back(std::move(dec));
        }
    }


    // ---- ANI ----
    for (const auto& src : m_loadedAnimations)
    {
        resource::DecodedAniData dec;
        std::string err;
        if (resource::AniDecoder::decode(dec, src.bytes, &err))
            m_decodedAni.emplace_back(std::move(dec));
        else
            Log::error(err);
    }

    // ---- SFX ----
    for (const auto& src : m_loadedSfx)
    {
        resource::DecodedSfxData dec;
        std::string err;
        if (resource::SfxDecoder::decode(dec, src.bytes, &err))
            m_decodedSfx.emplace_back(std::move(dec));
        else
            Log::error(err);
    }

    //Log::info("[AssetPipeline] DecodeAssets DONE");
}

void AssetPipeline::parseAssets()
{
    //Log::info("[AssetPipeline] ParseAssets BEGIN");

    std::string err;

    for (std::size_t i = 0; i < m_decodedModels.size(); ++i)
    {
        const auto& dec = m_decodedModels[i];
        resource::O3DParsed parsed;

        // 🔹 sicherstellen, dass Log-Ordner existiert
        std::filesystem::create_directories("Log");

        // 🔹 stabiler, eindeutiger Dump-Dateiname
        const std::string dumpPath =
            "Log/o3d_model_" + std::to_string(i) + ".dump.txt";

        // 🔹 Dump starten
        Log::enablePipelineLogging(dumpPath);
        Log::pipelineInfo("=== O3D DUMP START ===");
        Log::pipelineInfo("modelIndex=" + std::to_string(i));

        if (!asset::parser::ModelParser::parse(parsed, dec, &err))
        {
            Log::error(
                "[AssetPipeline] ModelParser failed: " +
                (err.empty() ? "<unknown>" : err)
                );

            Log::pipelineError(
                "ModelParser failed: " +
                (err.empty() ? "<unknown>" : err)
                );

            Log::pipelineInfo("=== O3D DUMP END ===");
            Log::disablePipelineLogging();   // ✅ IM FEHLERFALL
            continue;
        }

        // 🔹 optional: Parser-Ergebnis dumpen
        if (parsed.mesh.exists)
            Log::pipelineInfo("[RESULT] mesh parsed");
        else
            Log::pipelineInfo("[RESULT] no mesh part");

        Log::pipelineInfo("=== O3D DUMP END ===");
        Log::disablePipelineLogging();       // ✅ WICHTIG: AUCH IM ERFOLGSFALL

        // Optional: später speichern
        // m_parsedModels.emplace_back(std::move(parsed));
    }

    // 🔍 Debug / Analyse
    // if (parsed.mesh.exists)
    // {
    //     Log::info(
    //         "[AssetPipeline] Parsed mesh:"
    //         " chunks=" + std::to_string(parsed.mesh.chunks.size()) +
    //         " coverage=" + std::to_string(parsed.mesh.coverage) +
    //         " scheme=" + parsed.mesh.chosenScheme
    //         );
    // }
    // else
    // {
    //     Log::warn("[AssetPipeline] Parsed model has no mesh part");
    // }

    // Optional: später speichern in m_parsedModels
    // m_parsedModels.emplace_back(std::move(parsed));


    //Log::info("[AssetPipeline] ParseAssets DONE");
}

void AssetPipeline::convertAssets()
{
    Log::enablePipelineLogging(
        m_projectData.logPath + "/asset_convert.log"
        );
    Log::info("[AssetPipeline] ConvertAssets BEGIN");

    resource::ConverterSettings cs;
    cs.overwriteExisting = false;
    cs.modelToolPath = m_projectData.projectRoot + "/tools/model_converter.exe";
    cs.sfxTargetExtension = ""; // oder ".efs", wenn du willst

    resource::ImageConverter imgConv(cs);
    resource::SfxConverter   sfxConv(cs);
    resource::ModelConverter mdlConv(cs);

    // ---- Images → PNG ----
    for (const auto& img : m_loadedImages)
    {
        auto res = imgConv.convert(img);
        if (!res.ok)
        {
            Log::pipelineError(
                "IMAGE rel=" + img.relPath.string() +
                " src=" + img.sourcePath.string() +
                " reason=" + (res.error.empty() ? "<decode failed>" : res.error)
                );
        }
    }

    // ---- SFX → pass-through ----
    // for (const auto& sfx : m_loadedSfx)
    // {
    //     auto res = sfxConv.convert(sfx);
    //     if (!res.ok)
    //     {
    //         Log::pipelineError(
    //             "SFX rel=" + sfx.relPath.string() +
    //             " src=" + sfx.sourcePath.string() +
    //             " reason=" + (res.error.empty() ? "<decode failed>" : res.error)
    //             );
    //     }
    // }

    // ---- Models → GLB ----
    for (const auto& mdl : m_loadedModels)
    {
        auto res = mdlConv.convert(mdl);
        if (!res.ok)
        {
            Log::pipelineError(
                "Model rel=" + mdl.relPath.string() +
                " src=" + mdl.sourcePath.string() +
                " reason=" + (res.error.empty() ? "<decode failed>" : res.error)
                );
        }
    }

    Log::info("[AssetPipeline] ConvertAssets DONE");
}
