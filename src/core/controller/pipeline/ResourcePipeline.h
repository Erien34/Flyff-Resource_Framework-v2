#pragma once

#include <vector>
#include <unordered_map>

#include "core/controller/PipelineController.h"
#include "ProjectData.h"

// Resource
#include "core/resource/descriptors/DescriptorRegistry.h"
#include "core/resource/path/PathBuilder.h"
#include "core/resource/parser/UniversalParser.h"
#include "core/resource/serializer/SerializerRegistry.h"
#include "core/resource/scanner/PathScanner.h"

// Data
#include "resource/parse/TokenData.h"

namespace core
{

class ResourcePipeline : public PipelineController
{
public:
    explicit ResourcePipeline(ProjectData& projectData);

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
            LoadDescriptors,
            ScanPaths,
            BuildPaths,
            ParseFiles,
            SerializeData,
            BuildContext,
            BuildSnapshot,
            Done
        };

        Step step = Step::LoadDescriptors;
        std::unordered_map<
            std::string,
            resource::PathScanner::Group
            > scanPaths;
    };

    State m_state;

private:
    // =====================
    // Pipeline Steps
    // =====================
    void loadDescriptors();
    void scanPaths();
    void buildPaths();
    void parseFiles();
    void serializeData();

private:
    // =====================
    // Pipeline Data (vorher DataController)
    // =====================
    ProjectData& m_projectData;

    modules::descriptors::DescriptorRegistry m_descriptorRegistry;
    modules::serializer::SerializerRegistry  m_serializerRegistry;

    std::vector<FileEntry>       m_pathEntries;
    std::vector<data::TokenData> m_parsedTokens;
};

} // namespace core
