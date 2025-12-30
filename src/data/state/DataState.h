#pragma once

namespace core
{
enum class DataState
{
    NotInitialized,
    FirstRunRequired,
    Initializing,

    ResourcePipeline,
    AssetPipeline,
    SourcePipeline,
    RuntimePipeline,
    PluginInit,

    Done,
    Ready,
    Idle, Test
};
}
