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
};

} // namespace core
