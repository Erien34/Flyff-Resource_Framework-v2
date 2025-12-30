#pragma once

#include <string>

#include "DataState.h"
#include "ProjectData.h"
#include "ConfigManager.h"

// PipelineController base
#include "Core/Controller/PipelineController.h"

// Pipelines
#include "Core/Controller/pipeline/ResourcePipeline.h"
#include "Core/Controller/pipeline/AssetPipeline.h"

// (vorerst stubbar)
#include "Core/Controller/pipeline/SourcePipeline.h"
#include "Core/Controller/pipeline/RuntimePipeline.h"

namespace core
{

class DataController
{
public:
    DataController();

    bool initialize(const std::string& projectRoot);
    void update();

    void applyFirstRunConfig(
        const std::string& clientPath,
        const std::string& resourcePath,
        const std::string& sourcePath
        );

    DataState dataState() const { return m_dataState; }
    const ProjectData& projectData() const { return m_projectData; }

private:
    void process();
    void stepInitializing();
    void bootstrapProjectFolders();

    void setDataState(DataState s);

    void runPipeline(PipelineController& pipeline, DataState next);

private:
    // ================= CORE DATA =================
    std::string m_projectRoot;
    ProjectData m_projectData;

    DataState m_dataState = DataState::NotInitialized;

    ConfigManager* m_config = nullptr;

    // ================= PIPELINES =================
    ResourcePipeline m_resourcePipeline;
    AssetPipeline    m_assetPipeline;
    SourcePipeline   m_sourcePipeline;
    RuntimePipeline  m_runtimePipeline;
};

} // namespace core
