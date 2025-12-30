// RuntimePipeline.h
#pragma once
#include "Core/Controller/PipelineController.h"
#include "ProjectData.h"
namespace core {
class RuntimePipeline : public PipelineController {
public:
    explicit RuntimePipeline(ProjectData&) {}
protected:
    PipelineResult onUpdate() override { m_jobState = JobState::Done; return {JobState::Done,false}; }
};
}
