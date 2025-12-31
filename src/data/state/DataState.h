#pragma once

namespace core
{
enum class DataState
{
    NotInitialized,
    FirstRunRequired,
    Initializing,

    ResourcePipeline,
    AssetPipelineA,
    AssetPipelineB,
    SourcePipeline,
    RuntimePipeline,
    PluginInit,

    Done,
    Ready,
    Idle, Test
};
}
