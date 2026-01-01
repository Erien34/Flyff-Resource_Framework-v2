// SourcePipeline.cpp
#include "core/controller/pipeline/SourcePipeline.h"
#include "source/extract/SourceExtract.h"

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
        return { JobState::Running, false };

    case State::Step::SourceAssemble:
        assembleSource();
        m_state.step = State::Step::SourceValidate;
        return { JobState::Running, false };

    case State::Step::SourceValidate:
        validateSource();
        m_state.step = State::Step::SourceSnapshot;
        return { JobState::Running, false };

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
    using namespace core::source::extract;

    // 1) Input aus vorheriger Phase
    const auto& groups = m_groups; // aus Collect + Descriptor

    // 2) Facts-Zielcontainer (in-memory)
    m_extractFacts.clear();
    m_extractFacts.reserve(1024); // optional

    // 3) Extractor ausführen (einziger Einstiegspunkt)
    m_sourceExtractor.extract(groups, m_extractFacts);

    Log::info(
        "SourceExtract finished: facts=" +
        std::to_string(m_extractFacts.size())
        );
}

void SourcePipeline::assembleSource()
{
    m_sourceAssembler.assemble(
        m_extractFacts,
        m_meshRenderSpec
        );

    Log::info(
        "SourceAssemble finished: "
        "TriList=" + std::to_string(m_meshRenderSpec.draw.primitiveType ==
                         core::source::assemble::model::PrimitiveType::TriangleList) +
        " DIP=" + std::to_string(m_meshRenderSpec.draw.usesDrawIndexedPrimitive) +
        " PerSubMesh=" + std::to_string(m_meshRenderSpec.draw.perSubMesh) +
        " LOD=" + std::to_string(m_meshRenderSpec.draw.usesLOD) +
        " TexEx=" + std::to_string(m_meshRenderSpec.draw.usesMaterialVariants) +
        " MappingHints=" + std::to_string(m_meshRenderSpec.draw.mappingFieldLocations.size())
        );
}

void SourcePipeline::validateSource()
{
    const auto& d = m_meshRenderSpec.draw;

    auto fail = [&](const std::string& msg)
    {
        Log::error("SourceValidate FAILED: " + msg);
        // je nach deinem Framework: JobState::Failed / Exception / Flag setzen
        // m_state.step = State::Step::Done; ...
    };

    if (d.primitiveType != core::source::assemble::model::PrimitiveType::TriangleList)
        fail("Expected TriangleList primitive type (DrawCall.PrimitiveType.TriangleList missing).");

    if (!d.usesDrawIndexedPrimitive)
        fail("Expected DrawIndexedPrimitive usage (DrawCall.Invoke.DrawIndexedPrimitive missing).");

    if (!d.perSubMesh)
        fail("Expected per-submesh draw (DrawCall.PerSubMesh missing).");

    if (!d.hasMaterialBlockEvidence)
        fail("Expected MATERIAL_BLOCK evidence (SubMesh.Block.Struct + SubMesh.Block.ArrayRead missing).");

    if (d.mappingFieldLocations.empty())
        fail("Expected mapping hints for MATERIAL_BLOCK fields (DrawCall.Mapping.MaterialBlockField missing).");

    // Warnings (optional)
    if (!d.usesLOD)
        Log::warn("SourceValidate: LOD grouping (SetGroup) not detected.");
    if (!d.usesMaterialVariants)
        Log::warn("SourceValidate: TextureEx variant not detected.");

    Log::info("SourceValidate OK.");
}

void SourcePipeline::snapshotSource()
{
    // v1 placeholder
    // Hier später Output ins data/internal/... (Snapshot) schreiben

    Log::info("SourceSnapshot: placeholder (v1)");
}
