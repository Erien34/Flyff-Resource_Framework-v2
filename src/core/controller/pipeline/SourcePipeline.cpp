// SourcePipeline.cpp
#include "core/controller/pipeline/SourcePipeline.h"

#include "Log.h"

using namespace core;

SourcePipeline::SourcePipeline(ProjectData& projectData)
    : m_projectData(projectData)
{
}

void SourcePipeline::onReset()
{
    m_state.step = State::Step::SourceScan;

    m_descriptorRegistry.clear();
    m_sourceIndex.clear();
    m_groups.clear();

    m_error.clear();
}

PipelineResult SourcePipeline::onUpdate()
{
    switch (m_state.step)
    {
    case State::Step::SourceScan:
        scanSource();
        m_state.step = State::Step::SourceCollect;
        return { JobState::Running, true };

    case State::Step::SourceCollect:
        collectSource();
        m_state.step = State::Step::SourceExtract;
        return { JobState::Running, true };

    case State::Step::SourceExtract:
        extractSource();
        m_state.step = State::Step::SourceAssemble;
        return { JobState::Running, true };

    case State::Step::SourceAssemble:
        assembleSource();
        m_state.step = State::Step::SourceValidate;
        return { JobState::Running, true };

    case State::Step::SourceValidate:
        validateSource();
        m_state.step = State::Step::SourceSnapshot;
        return { JobState::Running, true };

    case State::Step::SourceSnapshot:
        snapshotSource();
        m_state.step = State::Step::Done;
        m_jobState = JobState::Done;
        return { JobState::Done, false };

    case State::Step::Done:
        return { JobState::Done, false };
    }

    m_jobState = JobState::Error;
    m_error = "SourcePipeline: invalid state";
    return { JobState::Error, false, m_error };
}

// =======================================================
// Step Implementierungen
// =======================================================

void SourcePipeline::scanSource()
{
    if (m_projectData.sourcePath.empty())
    {
        m_jobState = JobState::Error;
        m_error = "SourcePipeline::scanSource: projectData.sourcePath is empty";
        Log::error(m_error);
        return;
    }

    core::source::SourceScanner::Settings s;
    s.sourceRoot = m_projectData.sourcePath;

    core::source::SourceScanner scanner(s);
    scanner.scan(m_sourceIndex);

    Log::info(
        "SourceScan finished: files=" + std::to_string(m_sourceIndex.size())
    );
}

void SourcePipeline::collectSource()
{
    // 1) Load Source descriptors (collect rules)
    core::source::descriptor::SourceDescriptorLoader loader;
    std::string err;

    // Core rules
    if (!loader.loadAll(m_projectData.descriptorSourceCorePath, m_descriptorRegistry, &err))
    {
        // Non-fatal if you want: but I'd rather fail hard v1
        m_jobState = JobState::Error;
        m_error = "SourcePipeline::collectSource: loadAll(core) failed: " + err;
        Log::error(m_error);
        return;
    }

    // Plugin rules (optional): we can call loadAll again if you later implement "append mode".
    // v1 loader clears registry; so we SKIP plugin load here for now.
    // Sobald du JSON-Loader hast, bauen wir "loadAll(core, plugins)" als 1 Call.

    // 2) Build groups
    core::source::SourceCollector::Settings cs;
    cs.groupByBasename = true;
    cs.applyDescriptorRules = true;
    cs.keepFirstCategory = true;

    core::source::SourceCollector collector(cs);
    collector.collect(m_sourceIndex, m_descriptorRegistry, m_groups);

    Log::info(
        "SourceCollect finished: groups=" + std::to_string(m_groups.size())
    );
}

void SourcePipeline::extractSource()
{
    // v1 placeholder
    // Hier docken später SourceExtractBase + konkrete Extractors an (SubMeshRuleExtractor etc.)

    Log::info("SourceExtract: placeholder (v1)");
}

void SourcePipeline::assembleSource()
{
    // v1 placeholder
    // Hier mappen wir extrahierte Specs in das interne Tool-Model

    Log::info("SourceAssemble: placeholder (v1)");
}

void SourcePipeline::validateSource()
{
    // v1 placeholder
    // Hier Checks: fehlen Regeln? Gruppen leer? widersprüchliche Specs?

    Log::info("SourceValidate: placeholder (v1)");
}

void SourcePipeline::snapshotSource()
{
    // v1 placeholder
    // Hier später Output ins data/internal/... (Snapshot) schreiben

    Log::info("SourceSnapshot: placeholder (v1)");
}
