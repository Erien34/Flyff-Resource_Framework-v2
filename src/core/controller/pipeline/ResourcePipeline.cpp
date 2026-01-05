#include "controller/pipeline/ResourcePipeline.h"

#include "Log.h"
#include "core/resource/descriptors/DescriptorLoader.h"
#include "core/resource/serializer/SerializerBootstrap.h"
#include "resource/path/PathBuilder.h"

using namespace core;
using namespace modules::serializer;

ResourcePipeline::ResourcePipeline(ProjectData& projectData)
    : m_projectData(projectData)
{
}

void ResourcePipeline::onReset()
{
    m_state.step = State::Step::LoadDescriptors;
}

PipelineResult ResourcePipeline::onUpdate()
{
    switch (m_state.step)
    {
    case State::Step::LoadDescriptors:
        loadDescriptors();
        m_state.step = State::Step::ScanPaths;
        return { JobState::Running, true };

    case State::Step::ScanPaths:
        scanPaths();
        m_state.step = State::Step::BuildPaths;
        return { JobState::Running, true };

    case State::Step::BuildPaths:
        buildPaths();
        m_state.step = State::Step::ParseFiles;
        return { JobState::Running, true };

    case State::Step::ParseFiles:
        parseFiles();
        m_state.step = State::Step::SerializeData;
        return { JobState::Running, true };

    case State::Step::SerializeData:
        serializeData();
        m_state.step = State::Step::BuildContext;
        return { JobState::Running, true };

    case State::Step::BuildContext:
        // später: ResourceContext bauen
        m_state.step = State::Step::BuildSnapshot;
        return { JobState::Running, true };

    case State::Step::BuildSnapshot:
        // später: Snapshot schreiben
        m_state.step = State::Step::Done;
        m_jobState = JobState::Done;
        return { JobState::Done, false };

    case State::Step::Done:
        return { JobState::Done, false };
    }

    m_jobState = JobState::Error;
    m_error = "ResourcePipeline: invalid state";
    return { JobState::Error, false, m_error };
}

// =======================================================
// Step Implementierungen (1:1 aus DataController)
// =======================================================

void ResourcePipeline::loadDescriptors()
{
    bool ok = m_descriptorRegistry.loadAll(
        m_projectData.descriptorResourceCorePath,
        m_projectData.descriptorResourcePluginPath);

    if (!ok)
    {
        Log::error("Descriptor loading failed");
        return;
    }

    registerCoreSerializers(m_serializerRegistry);

    Log::info("Descriptors loaded");
}

void ResourcePipeline::scanPaths()
{
    resource::PathScanner scanner;

    resource::PathScanner::Settings s;
    s.clientRoot   = m_projectData.clientPath;
    s.resourceRoot = m_projectData.resourcePath;
    s.worldFolderName = "World";

    m_state.scanPaths = scanner.scan(s);

    // Optional: Logging (sehr empfohlen)
    Log::info(
        "[ResourcePipeline] PathScanner: " +
        std::to_string(m_state.scanPaths.size()) +
        " world groups detected"
        );
}

void ResourcePipeline::buildPaths()
{
    // 1) ZUERST: World-Dateien einsammeln
    for (const auto& [domain, group] : m_state.scanPaths)
    {
        for (const auto& e : group.clientFiles)
            PathBuilder::addFile(
                e.absPath,
                e.domain,
                FileEntry::Source::Client
                );

        for (const auto& e : group.resourceFiles)
            PathBuilder::addFile(
                e.absPath,
                e.domain,
                FileEntry::Source::Resource
                );
    }

    // 2) DANACH: Deskriptor-Dateien + World-Dateien zusammenbauen
    auto descriptors = m_descriptorRegistry.all();

    std::vector<modules::descriptors::Descriptor> list;
    for (auto& [_, d] : descriptors)
        list.push_back(d);

    m_pathEntries = PathBuilder::build(
        m_projectData.resourcePath,
        list
        );

    Log::info(
        "BuildPaths found " +
        std::to_string(m_pathEntries.size()) +
        " files"
        );
}

void ResourcePipeline::parseFiles()
{
    modules::parser::UniversalParser parser;

    m_parsedTokens.clear();

    for (const auto& file : m_pathEntries)
    {
        data::TokenData tokens = parser.parse(
            file,
            m_projectData.resourcePath
            );

        if (tokens.tokens.empty())
            continue;

        m_parsedTokens.emplace_back(std::move(tokens));

    }
}

void ResourcePipeline::serializeData()
{

    // 1) Tokens nach ModuleId gruppieren
    std::unordered_map<std::string, std::vector<data::TokenData>> grouped;

    for (const auto& tokenData : m_parsedTokens)
    {
        grouped[tokenData.moduleId].push_back(tokenData);
    }

    // 2) Für jedes Modul passenden Serializer ausführen
    for (auto& [moduleId, tokenGroup] : grouped)
    {
        SerializerBase* serializer = m_serializerRegistry.get(moduleId);

        if (!serializer)
        {
            Log::warn("No serializer registered for module: " + moduleId);
            continue;
        }

        // Log::info(
        //     "Serializing module [" + moduleId + "] with " +
        //     std::to_string(tokenGroup.size()) + " files"
        //     );

        serializer->run(tokenGroup);
    }

    Log::info("SerializeData finished");
}
