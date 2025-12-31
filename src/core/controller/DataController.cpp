#include "DataController.h"
#include "Log.h"

#include <filesystem>
namespace fs = std::filesystem;

namespace core
{

DataController::DataController()
    : m_resourcePipeline(m_projectData)
    , m_assetPipelineA(m_projectData)
    , m_assetPipelineB(m_projectData)
    , m_sourcePipeline(m_projectData)   // stub ok
    , m_runtimePipeline(m_projectData)  // stub ok
{
}

bool DataController::initialize(const std::string& projectRoot)
{
    m_projectRoot = projectRoot;
    setDataState(DataState::Initializing);
    return true;
}

void DataController::applyFirstRunConfig(
    const std::string& c,
    const std::string& r,
    const std::string& s)
{
    if (!m_config)
        return;

    m_config->setClientPath(c);
    m_config->setResourcePath(r);
    m_config->setSourcePath(s);
    m_config->save();

    setDataState(DataState::Initializing);
}

void DataController::update()
{
    process();
}

void DataController::setDataState(DataState s)
{
    m_dataState = s;
    // Kein JobState-Reset mehr nötig, Pipelines kapseln ihren Zustand selbst
}

void DataController::process()
{
    switch (m_dataState)
    {
    case DataState::Initializing:
        stepInitializing();
        break;

    case DataState::FirstRunRequired:
        // Warten auf UI (DialogFirstRun)
        break;

    case DataState::ResourcePipeline:
        runPipeline(m_resourcePipeline, DataState::AssetPipelineA);
        break;

    case DataState::AssetPipelineA:
        runPipeline(m_assetPipelineA, DataState::SourcePipeline);
        break;

    case DataState::SourcePipeline:
        runPipeline(m_sourcePipeline, DataState::AssetPipelineB);
        break;

    case DataState::AssetPipelineB:
        runPipeline(m_assetPipelineB, DataState::RuntimePipeline);
        break;

    case DataState::RuntimePipeline:
        runPipeline(m_runtimePipeline, DataState::PluginInit);
        break;

    case DataState::PluginInit:
        // Plugins initialisieren (später)
        setDataState(DataState::Ready);
        break;

    case DataState::Ready:
        setDataState(DataState::Idle);
        break;

    case DataState::Idle:
        // nichts tun, warten auf User / UI
        break;

    default:
        break;
    }
}

void DataController::runPipeline(PipelineController& pipeline, DataState next)
{
    PipelineResult res = pipeline.update();

    if (res.state == JobState::Error)
    {
        Log::error(std::string("[DataController] Pipeline error: ") +
                   (res.error.empty() ? "<unknown>" : res.error));
        // Optional: setDataState(DataState::Idle) oder eigener Error-State
        return;
    }

    if (res.state == JobState::Done)
    {
        pipeline.reset();
        setDataState(next);
    }
}

void DataController::stepInitializing()
{
    Log::info("stepInitializing() called");

    if (!m_config)
        m_config = new ConfigManager(m_projectRoot);

    if (!m_config->load())
    {
        setDataState(DataState::FirstRunRequired);
        return;
    }

    m_projectData.projectRoot  = m_projectRoot;
    m_projectData.clientPath   = m_config->clientPath();
    m_projectData.resourcePath = m_config->resourcePath();
    m_projectData.sourcePath   = m_config->sourcePath();

    if (m_projectData.clientPath.empty() ||
        m_projectData.resourcePath.empty())
    {
        setDataState(DataState::FirstRunRequired);
        return;
    }

    bootstrapProjectFolders();

    m_projectData.valid = true;

    Log::info("Initialization completed");
    setDataState(DataState::ResourcePipeline);
}

void DataController::bootstrapProjectFolders()
{
    // ================= BASE =================
    m_projectData.configPath       = m_projectRoot + "/config";
    m_projectData.logPath          = m_projectRoot + "/log";
    m_projectData.dataPath         = m_projectRoot + "/data";
    m_projectData.internalDataPath = m_projectRoot + "/data/internal";
    m_projectData.externalDataPath = m_projectRoot + "/data/external";
    m_projectData.assetCachePath   = m_projectRoot + "/assets";
    m_projectData.pluginPath       = m_projectRoot + "/plugins";

    // ================= DESCRIPTORS =================
    const std::string descriptorDir =
        m_projectData.internalDataPath + "/descriptors";

    // ---- Resource ----
    m_projectData.descriptorResourceCorePath =
        descriptorDir + "/resource/core";
    m_projectData.descriptorResourcePluginPath =
        descriptorDir + "/resource/plugins";

    // ---- Source ----
    m_projectData.descriptorSourceCorePath =
        descriptorDir + "/source/core";
    m_projectData.descriptorSourcePluginPath =
        descriptorDir + "/source/plugins";

    // ---- Runtime (future) ----
    m_projectData.descriptorRuntimeCorePath =
        descriptorDir + "/runtime/core";
    m_projectData.descriptorRuntimePluginPath =
        descriptorDir + "/runtime/plugins";

    // ================= CREATE DIRECTORIES (idempotent) =================
    fs::create_directories(m_projectData.configPath);
    fs::create_directories(m_projectData.logPath);
    fs::create_directories(m_projectData.internalDataPath);
    fs::create_directories(m_projectData.externalDataPath);
    fs::create_directories(m_projectData.assetCachePath);
    fs::create_directories(m_projectData.pluginPath);

    fs::create_directories(descriptorDir);

    // Resource
    fs::create_directories(m_projectData.descriptorResourceCorePath);
    fs::create_directories(m_projectData.descriptorResourcePluginPath);

    // Source
    fs::create_directories(m_projectData.descriptorSourceCorePath);
    fs::create_directories(m_projectData.descriptorSourcePluginPath);

    // Runtime
    fs::create_directories(m_projectData.descriptorRuntimeCorePath);
    fs::create_directories(m_projectData.descriptorRuntimePluginPath);
}

} // namespace core
