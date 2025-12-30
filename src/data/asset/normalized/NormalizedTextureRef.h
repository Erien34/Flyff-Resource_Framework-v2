#pragma once
#include <string>

struct NormalizedTextureRef
{
    std::string name;          // logical name (e.g. "Body_Diffuse")
    std::string sourceAsset;   // original asset id or filename (debug/reference)
    
    // semantic usage
    enum class Usage
    {
        Diffuse,
        Normal,
        Specular,
        Emissive,
        Opacity,
        Unknown
    } usage = Usage::Unknown;
};