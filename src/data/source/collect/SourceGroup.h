#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include "data/source/index/SourceIndexList.h"

namespace data::source
{

struct SourceGroup
{
    // Primärer Group-Key (v1): Basename / Unit Name
    // z.B. "Object3D", "WND_Inventory"
    std::string id;

    // optionaler Kontext (nicht Gameplay-Semantik)
    // z.B. "UI", "Server"
    std::string category;

    // optional: welche Descriptor-IDs haben gematcht
    std::vector<std::string> matchedDescriptorIds;

    // alle Dateien, die zu dieser Gruppe gehören (.h/.cpp/.hpp/.inl)
    std::vector<SourceFileEntry> files;

    // gemeinsamer Ordner (best-effort)
    std::filesystem::path baseDirectory;
};

} // namespace data::source
