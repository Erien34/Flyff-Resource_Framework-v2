#pragma once
#include <string>

namespace resource { struct O3DDecoded; struct O3DParsed; }

namespace asset::parser
{
    class ModelParser
    {
    public:
        // Strukturelle Analyse: Header/Chunks/Blöcke erkennen – keine Interpretation.
        static bool parse(resource::O3DParsed& out, const resource::O3DDecoded& in, std::string* outError = nullptr);
    };
}
