#pragma once

#include <vector>

#include "core/controller/PipelineController.h"
#include "ProjectData.h"

// Descriptor (Source)
#include "core/source/descriptor/SourceDescriptorLoader.h"
#include "core/source/descriptor/SourceDescriptorRegistry.h"

// Logic
#include "core/source/scan/SourceScanner.h"
#include "core/source/collect/SourceCollector.h"

// Data
#include "data/source/index/SourceIndexList.h"
#include "data/source/collect/SourceGroup.h"

#include "core/source/extract/SourceExtract.h"
#include "data/source/extract/ExtractFact.h"

#include "core/source/assemble/SourceAssembler.h"
#include "data/source/spec/MeshRenderSpec.h"

namespace core
{

class SourcePipeline : public PipelineController
{
public:
    explicit SourcePipeline(ProjectData& projectData);

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
            SourceScan,
            SourceCollect,
            SourceExtract,
            SourceAssemble,
            SourceValidate,
            SourceSnapshot,
            Done
        };

        Step step = Step::SourceScan;
    };

    State m_state;

private:
    // =====================
    // Pipeline Steps
    // =====================
    void scanSource();
    void collectSource();
    void extractSource();     // v1: placeholder
    void assembleSource();    // v1: placeholder
    void validateSource();    // v1: placeholder
    void snapshotSource();    // v1: placeholder

private:
    // =====================
    // Pipeline Data
    // =====================
    ProjectData& m_projectData;

    // loaded rules for collect
    core::source::descriptor::SourceDescriptorRegistry m_descriptorRegistry;

    // scan result
    data::source::SourceIndexList m_sourceIndex;

    // collect result
    std::vector<data::source::SourceGroup> m_groups;

    // extract result (facts)
    std::vector<core::source::extract::ExtractFact> m_extractFacts;

    // extractor (owns schema extractors)
    core::source::extract::SourceExtractor m_sourceExtractor;

    core::source::assemble::SourceAssembler m_sourceAssembler;
    core::source::assemble::model::MeshRenderSpec m_meshRenderSpec;
};

} // namespace core
