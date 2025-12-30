#pragma once

#include <string>
#include "data/state/JobState.h"

using namespace core;

struct PipelineResult
{

    JobState state = JobState::Idle;
    bool progressed = false;
    std::string error;
};

class PipelineController
{
public:
    virtual ~PipelineController() = default;

    PipelineResult update()
    {
        if (m_jobState == JobState::Done ||
            m_jobState == JobState::Error)
        {
            return { m_jobState, false, m_error };
        }

        m_jobState = JobState::Running;
        return onUpdate();
    }

    virtual void reset()
    {
        m_jobState = JobState::Idle;
        m_error.clear();
        onReset();
    }

    JobState jobState() const { return m_jobState; }
    const std::string& error() const { return m_error; }

protected:
    virtual PipelineResult onUpdate() = 0;
    virtual void onReset() {}

protected:
    JobState m_jobState = JobState::Idle;
    std::string m_error;
};
