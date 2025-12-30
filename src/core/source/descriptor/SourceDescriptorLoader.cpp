#include "SourceDescriptorLoader.h"

#include <filesystem>

namespace fs = std::filesystem;
using namespace core::source::descriptor;

bool SourceDescriptorLoader::loadAll(
    const std::string& directory,
    SourceDescriptorRegistry& outRegistry,
    std::string* error)
{
    outRegistry.clear();

    if (!fs::exists(directory))
    {
        if (error)
            *error = "SourceDescriptor directory not found: " + directory;
        return false;
    }

    // ===============================
    // Default / Core Rules (v1)
    // ===============================

    // --- Pair .h / .cpp ---
    SourceDescriptor pairDesc;
    pairDesc.id = "pair_basename";
    pairDesc.rules.push_back({
        SourceRuleType::PairBasename,
        "",
        { ".h", ".hpp", ".cpp", ".inl" },
        ""
    });

    // --- UI (WND_*) ---
    SourceDescriptor uiDesc;
    uiDesc.id = "ui";
    uiDesc.rules.push_back({
        SourceRuleType::Prefix,
        "WND_",
        {},
        "UI"
    });

    // --- Server (WorldServer) ---
    SourceDescriptor serverDesc;
    serverDesc.id = "server";
    serverDesc.rules.push_back({
        SourceRuleType::DirectoryContains,
        "WorldServer",
        {},
        "Server"
    });

    outRegistry.registerDescriptor(std::move(pairDesc));
    outRegistry.registerDescriptor(std::move(uiDesc));
    outRegistry.registerDescriptor(std::move(serverDesc));

    return true;
}
