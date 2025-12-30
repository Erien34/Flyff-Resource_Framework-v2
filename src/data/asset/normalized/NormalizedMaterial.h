#pragma once
#include <string>
#include <vector>
#include "NormalizedTextureRef.h"

struct NormalizedMaterial
{
    std::string name;

    // simple PBR-ready values
    float baseColor[4]   = {1,1,1,1};
    float metallic       = 0.0f;
    float roughness      = 1.0f;
    bool  doubleSided    = false;

    // texture slots (logical!)
    std::vector<NormalizedTextureRef> textures;
};