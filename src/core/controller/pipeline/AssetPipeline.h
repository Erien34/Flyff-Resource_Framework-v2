#pragma once

#include <vector>

#include "Core/Controller/PipelineController.h"
#include "ProjectData.h"

// ================= ASSET INDEX =================
#include "core/asset/index/AssetIndexBuilder.h"

// ================= ASSET SOURCES =================
#include "data/asset/source/ModelSource.h"
#include "asset/source/ImageSource.h"
#include "asset/source/AnimationSource.h"
#include "asset/source/SfxSource.h"

// ================= DECODED DATA =================
#include "data/asset/decoded/O3DDecoded.h"
#include "data/asset/decoded/DecodedImageData.h"
#include "data/asset/decoded/DecodedAniData.h"
#include "data/asset/decoded/DecodedSfxData.h"

namespace core
{

class AssetPipeline : public PipelineController
{
public:
    explicit AssetPipeline(ProjectData& projectData);

protected:
    PipelineResult onUpdate() override;
    void onReset() override;

private:
    // =====================
    // Pipeline State
    // =====================
    struct State
    {
        enum class Step
        {
            ScanAssets,
            BuildDirectories,
            LoadAssets,
            DecryptAssets,
            DecodeAssets,
            ParseAssets,
            ConvertAssets,
            BuildSnapshot,
            Done
        };

        Step step = Step::ScanAssets;
    };

    State m_state;

private:
    // =====================
    // Pipeline Steps
    // =====================
    void scanAssets();
    void buildAssetDirectories();
    void loadAssets();
    void decryptAssets();
    void decodeAssets();
    void parseAssets();
    void convertAssets();

private:
    // =====================
    // Pipeline Data (vorher DataController)
    // =====================
    ProjectData& m_projectData;

    resource::AssetIndexBuilder::Index m_assetIndex;
    bool m_assetIndexBuilt = false;

    // ---- Loaded ----
    std::vector<resource::ModelSource>     m_loadedModels;
    std::vector<resource::ImageSource>     m_loadedImages;
    std::vector<resource::AnimationSource> m_loadedAnimations;
    std::vector<resource::SfxSource>       m_loadedSfx;

    // ---- Decoded ----
    std::vector<resource::O3DDecoded>       m_decodedModels;
    std::vector<resource::DecodedImageData> m_decodedImages;
    std::vector<resource::DecodedAniData>   m_decodedAni;
    std::vector<resource::DecodedSfxData>   m_decodedSfx;
};

} // namespace core
