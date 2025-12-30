#pragma once
#include <vector>
#include <string>
#include "data/module/FileEntry.h"
#include "resource/descriptors/Descriptor.h"

class PathBuilder
{
public:
    static std::vector<FileEntry> build(
        const std::string& resourceRoot,
        const std::vector<modules::descriptors::Descriptor>& descriptors);
};
