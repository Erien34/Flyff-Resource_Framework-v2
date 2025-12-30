#pragma once
#include "core/controller/PipelineController.h"

class ResourcePipelineController : public PipelineController
{
protected:
    PipelineResult onUpdate() override;
    void onReset() override;

private:
    struct State
    {
        enum class Step
        {
            LoadDescriptors,
            BuildPaths,
            ParseFiles,
            SerializeData,
            BuildContext,
            BuildSnapshot,
            Done
        };

        Step step = Step::LoadDescriptors;
    };

    State m_state;

private:
    void LoadDescriptors();
    void BuildPaths();
    void ParseFiles();
    void SerializeData();
};
