#pragma once

#include <string>
#include <vector>

namespace core::source::descriptor
{

    enum class SourceRuleType
    {
        PairBasename,        // .h + .cpp zusammenfassen
        Prefix,              // z.B. WND_
        DirectoryContains    // z.B. WorldServer
    };

    struct SourceRule
    {
        SourceRuleType type;

        // Für Prefix / DirectoryContains
        std::string value;

        // Für PairBasename
        std::vector<std::string> extensions;

        // rein organisatorisch (z.B. UI / Server)
        // KEINE Gameplay-Semantik
        std::string category;
    };

    struct SourceDescriptor
    {
        std::string id;                  // z.B. "ui", "server", "default"
        std::vector<SourceRule> rules;
    };

} // namespace core::source::descriptor
