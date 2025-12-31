#pragma once
#include <string>

namespace asset { struct O3DDecoded; struct O3DParsed; }

namespace asset::parser
{
    class ModelParser
    {
    public:
        // Strukturelle Analyse: Header/Chunks/Blöcke erkennen – keine Interpretation.
        static bool parse(asset::O3DParsed& out, const asset::O3DDecoded& in, std::string* outError = nullptr);
    };
}
