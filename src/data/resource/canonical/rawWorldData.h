#pragma once
#include <string>
#include <unordered_map>
#include <vector>

namespace data::module::rawworld
{
struct WorldSettings
{
    int indoor = 0;
    uint32_t ambient = 0;
    uint32_t bgColor = 0;
    int fly = 0;
    float camPos[3]{};
    float camRot[3]{};
};

struct Region
{
    int type = 0;
    int id = 0;
    float x1=0, y1=0, x2=0, y2=0;
    std::string script;
};

struct WorldArea
{
    std::string key;
    WorldSettings settings;
    std::vector<Region> regions;
    std::string title;
    std::string description;
};

struct rawWorldData
{
    std::unordered_map<std::string, WorldArea> areas;
};
}
