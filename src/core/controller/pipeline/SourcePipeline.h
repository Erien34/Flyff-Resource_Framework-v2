// SourcePipeline.h
#pragma once
#include "Core/Controller/PipelineController.h"
#include "ProjectData.h"
namespace core {
class SourcePipeline : public PipelineController {
public:
    explicit SourcePipeline(ProjectData&) {}
protected:
    PipelineResult onUpdate() override { m_jobState = JobState::Done; return {JobState::Done,false}; }
};
}
