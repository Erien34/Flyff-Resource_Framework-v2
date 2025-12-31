#pragma once

#include "core/controller/PipelineController.h"
#include "ProjectData.h"

// ===== Index =====
#include "core/asset/index/AssetIndexBuilder.h"

// ===== Sources =====
#include "data/asset/source/ModelSource.h"
#include "data/asset/source/ImageSource.h"
#include "data/asset/source/AnimationSource.h"
#include "data/asset/source/SfxSource.h"

// ===== Decoded =====
#include "data/asset/decoded/O3DDecoded.h"
#include "data/asset/decoded/DecodedImageData.h"
#include "data/asset/decoded/DecodedAniData.h"
#include "data/asset/decoded/DecodedSfxData.h"

// ===== Parsed =====
#include "data/asset/parsed/O3DParsed.h"

namespace core::pipeline
{

// =======================================================
// Phase A – LOAD / DECODE / PARSE (NO SPECS)
// =======================================================
class AssetPipelineA : public PipelineController
{
public:
    explicit AssetPipelineA(ProjectData& project);

protected:
    PipelineResult onUpdate() override;
    void onReset() override;

private:
    enum class Step
    {
        ScanAssets,
        BuildDirectories,
        LoadAssets,
        DecryptAssets,
        DecodeAssets,
        ParseAssets,
        Done
    };

    Step m_step = Step::ScanAssets;
    ProjectData& m_projectData;

    ::asset::AssetIndexBuilder::Index m_assetIndex;

    // -------- DATA --------
    std::vector<::asset::ModelSource>     m_loadedModels;
    std::vector<::asset::ImageSource>     m_loadedImages;
    std::vector<::asset::AnimationSource> m_loadedAnimations;
    std::vector<::asset::SfxSource>       m_loadedSfx;

    std::vector<::asset::O3DDecoded>       m_decodedModels;
    std::vector<::asset::DecodedImageData> m_decodedImages;
    std::vector<::asset::DecodedAniData>   m_decodedAni;
    std::vector<::asset::DecodedSfxData>   m_decodedSfx;

    std::vector<::asset::O3DParsed> m_parsedModels;

    // -------- STEPS --------
    void scanAssets();
    void buildAssetDirectories();
    void loadAssets();
    void decryptAssets();
    void decodeAssets();
    void parseAssets();
};


// =======================================================
// Phase B – APPLY SPECS / CONVERT
// =======================================================
class AssetPipelineB : public PipelineController
{
public:
    explicit AssetPipelineB(ProjectData& project);

protected:
    PipelineResult onUpdate() override;
    void onReset() override;

private:
    enum class Step
    {
        ConvertAssets,
        BuildSnapshot,
        Done
    };

    Step m_step = Step::ConvertAssets;
    ProjectData& m_projectData;

    void convertAssets();
    void buildSnapshot();
};

} // namespace asset::pipeline
