#pragma once

#include <string>
#include <vector>
#include "Descriptor.h"
#include "json.hpp"

namespace modules::descriptors
{

class DescriptorLoader
{
public:
    // ============================
    // Static API (wie im CPP benutzt)
    // ============================

    static void bootstrapCoreIfNeeded(const std::string& coreDir);

    static std::vector<Descriptor>
    loadFromDirectory(const std::string& dir, const std::string& scope);

    static Descriptor loadFromFile(
            const std::string& path,
            const std::string& scope);

private:
    // ============================
    // Helper – nur Signaturen
    // ============================
    static bool hasAnyJson(const std::string& dir);

    static void writeFileIfMissing(
        const std::string& dir,
        const std::string& file,
        const std::string& content
        );

    // JSON-Helfer (Signatur muss zum CPP passen)
    static Descriptor::AssetsRequired readAssetsRequired(const nlohmann::json& j);
    static Descriptor::ParserRule readParserRule(const nlohmann::json& j);
};

} // namespace modules::descriptors
