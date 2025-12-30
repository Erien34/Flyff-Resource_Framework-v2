#include "DescriptorLoader.h"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace modules::descriptors
{
static Descriptor::AssetsRequired readAssetsRequired(const json& j)
{
    Descriptor::AssetsRequired a{};
    if (!j.is_object())
        return a;

    a.icon      = j.value("icon", false);
    a.model     = j.value("model", false);
    a.animation = j.value("animation", false);
    a.effect    = j.value("effect", false);
    a.sound     = j.value("sound", false);
    return a;
}

static Descriptor::ParserRule readParserRule(const json& j)
{
    Descriptor::ParserRule r{};
    r.pattern = j.value("pattern", "");
    r.parserId = j.value("parserId", 0);
    r.priority = j.value("priority", 0);
    return r;
}

bool DescriptorLoader::hasAnyJson(const std::string& dir)
{
    if (!fs::exists(dir)) return false;
    for (auto& e : fs::directory_iterator(dir))
    {
        if (e.is_regular_file() && e.path().extension() == ".json")
            return true;
    }
    return false;
}

void DescriptorLoader::writeFileIfMissing(const std::string& dir, const std::string& filename, const std::string& content)
{
    fs::create_directories(dir);
    fs::path p = fs::path(dir) / filename;

    if (fs::exists(p))
        return;

    std::ofstream out(p.string(), std::ios::binary);
    if (!out) return;
    out << content;
}

// ----------------------------
// BOOTSTRAP (CORE)
// ----------------------------
void DescriptorLoader::bootstrapCoreIfNeeded(const std::string& coreDir)
{
    fs::create_directories(coreDir);

    // Wenn bereits irgendwas existiert: nicht überschreiben
    if (hasAnyJson(coreDir))
        return;

    // NOTE:
    // - id:            V2 key
    // - serializerKey: registry key, NICHT Klassenname
    // - contextBuilderKey: registry key
    // - resourceFiles: echte Dateien (dein V1-Ansatz)
    // - parserRules: wie in V1
    //
    // Du kannst später optional "sourceFiles"/"export"/etc hinzufügen.

    // ----------------------------
    // DEFINES
    // ----------------------------
    writeFileIfMissing(coreDir, "defines.json", R"json(
{
  "id": "defines",
  "name": "Defines",
  "scope": "core",
  "resourceFiles": [
    { "file": "defineItem.h",            "domain": "item" },
    { "file": "defineitemkind.h",        "domain": "item" },
    { "file": "defineItemType.h",        "domain": "item" },
    { "file": "defineitemgrade.h",       "domain": "item" },

    { "file": "defineSkill.h",           "domain": "skill" },
    { "file": "defineText.h",            "domain": "text" },

    { "file": "defineWorld.h",           "domain": "world" },
    { "file": "ContinentDef.h",          "domain": "world" },

    { "file": "defineAttribute.h",       "domain": "attribute" },
    { "file": "defineObj.h",             "domain": "object" },
    { "file": "defineMapComboBoxData.h", "domain": "ui" },
    { "file": "ResData.h",               "domain": "ui" },

    { "file": "defineJob.h",             "domain": "job" },
    { "file": "defineHonor.h",           "domain": "job" },

    { "file": "definequest.h",           "domain": "quest" },
    { "file": "defineSound.h",           "domain": "sound" },

    { "file": "defineEvent.h",           "domain": "event" },
    { "file": "defineEventArena.h",      "domain": "event" },

    { "file": "defineNeuz.h",             "domain": "engine" },

    { "file": "define.h",                "domain": "global" },
    { "file": "lang.h",                  "domain": "global" }
  ],
  "parserRules": [
    { "pattern": "*.h", "parserId": 0, "priority": 0 }
  ],
  "serializerKey": "define",
  "contextBuilderKey": "define",
  "dependencies": [],
  "type": "define",
  "assetsRequired": {
    "icon": false,
    "model": false,
    "animation": false,
    "effect": false,
    "sound": false
  },
  "outputModel": "rawDefineData",
  "engineHook": ""
}
)json");

    // ----------------------------
    // TEXT
    // ----------------------------
    writeFileIfMissing(coreDir, "text.json", R"json(
{
  "id": "text",
  "name": "Text / Strings",
  "scope": "core",

  "resourceFiles": [

    { "file": "textClient.txt.txt",  "domain": "ui" },
    { "file": "resData.txt.txt",     "domain": "ui" },
    { "file": "textEmotion.txt.txt", "domain": "ui" },

    { "file": "propItem.txt.txt",        "domain": "item" },
    { "file": "propItemEtc.txt.txt",     "domain": "item" },

    { "file": "propSkill.txt.txt",        "domain": "skill" },
    { "file": "propTroupeSkill.txt.txt", "domain": "skill" },

    { "file": "character.txt.txt",        "domain": "job" },
    { "file": "character-etc.txt.txt",    "domain": "job" },
    { "file": "character-school.txt.txt", "domain": "job" },
    { "file": "honorList.txt.txt",         "domain": "job" }
  ],

  "parserRules": [
    { "pattern": "*.inc", "parserId": 0, "priority": 5 },
    { "pattern": "*.txt", "parserId": 0, "priority": 0 }
  ],

  "serializerKey": "text",
  "contextBuilderKey": "text",
  "dependencies": [ "defines" ],
  "type": "text",

  "assetsRequired": {
    "icon": false,
    "model": false,
    "animation": false,
    "effect": false,
    "sound": false
  },

  "outputModel": "rawTextData",
  "engineHook": ""
}
)json");

    // ----------------------------
    // ITEMS
    // ----------------------------
    writeFileIfMissing(coreDir, "items.json", R"json(
{
  "id": "items",
  "name": "Items",
  "scope": "core",
  "resourceFiles": [
    { "file": "propItem.txt",        "domain": "item" },
    { "file": "propItemEtc.inc",     "domain": "item" },
    { "file": "propPackItem.inc",    "domain": "item" },
    { "file": "TreasureItem.txt",        "domain": "item" },
    { "file": "TresureItem.txt",         "domain": "item" },
    { "file": "TreasureCombine.txt",     "domain": "item" },
    { "file": "TresureCombine.txt",      "domain": "item" },
    { "file": "teleportscroll.txt",      "domain": "item" },
    { "file": "propGiftbox.inc",     "domain": "item" }
  ],
  "parserRules": [
    { "pattern": "*.txt", "parserId": 0, "priority": 0 },
    { "pattern": "*.inc", "parserId": 0, "priority": 0 }
  ],
  "serializerKey": "item",
  "contextBuilderKey": "item",
  "dependencies": [ "defines", "text" ],
  "type": "item",
  "assetsRequired": {
    "icon": true,
    "model": true,
    "animation": false,
    "effect": false,
    "sound": false
  },
  "outputModel": "rawItemData",
  "engineHook": "applyItemMechanics"
}
)json");

    // ----------------------------
    // SKILLS
    // ----------------------------
    writeFileIfMissing(coreDir, "skills.json", R"json(
{
  "id": "skills",
  "name": "Skills",
  "scope": "core",
  "resourceFiles": [
    { "file": "propSkill.txt",        "domain": "skill" },
    { "file": "propSkillAdd.csv",     "domain": "skill" },
    { "file": "propTroupeSkill.txt",  "domain": "skill" }
  ],
  "parserRules": [
    { "pattern": "*.txt", "parserId": 0, "priority": 0 },
    { "pattern": "*.csv", "parserId": 0, "priority": 0 }
  ],
  "serializerKey": "skill",
  "contextBuilderKey": "skill",
  "dependencies": [ "defines", "text", "items" ],
  "type": "skill",
  "assetsRequired": {
    "icon": true,
    "model": false,
    "animation": false,
    "effect": true,
    "sound": false
  },
  "outputModel": "rawSkillData",
  "engineHook": "applySkillMechanics"
}
)json");


    // ----------------------------
    // MONSTER
    // ----------------------------
    writeFileIfMissing(coreDir, "monster.json", R"json(
{
  "id": "monster",
  "name": "Monster",
  "scope": "core",
  "resourceFiles": [
    { "file": "propMover.txt",   "domain": "monster" },
    { "file": "propMoverEx.inc", "domain": "monster" }
  ],
  "parserRules": [
    { "pattern": "*.txt", "parserId": 0, "priority": 0 },
    { "pattern": "*.inc", "parserId": 0, "priority": 0 }
  ],
  "serializerKey": "monster",
  "contextBuilderKey": "monster",
  "dependencies": [ "defines", "text", "items", "skills" ],
  "type": "monster",
  "assetsRequired": {
    "icon": false,
    "model": true,
    "animation": true,
    "effect": false,
    "sound": false
  },
  "outputModel": "rawMonsterData",
  "engineHook": "applyMonsterMechanics"
}
)json");


    // ----------------------------
    // AI
    // ----------------------------
    writeFileIfMissing(coreDir, "ai.json", R"json(
{
  "id": "ai",
  "name": "AI / Behaviour",
  "scope": "core",
  "resourceFiles": [
    { "file": "propMoverAI.txt", "domain": "ai" },
    { "file": "propAIEvent.inc", "domain": "ai" }
  ],
  "parserRules": [
    { "pattern": "*.txt", "parserId": 0, "priority": 0 },
    { "pattern": "*.inc", "parserId": 0, "priority": 0 }
  ],
  "serializerKey": "ai",
  "contextBuilderKey": "ai",
  "dependencies": [ "monster", "skills", "defines" ],
  "type": "ai",
  "assetsRequired": {
    "icon": false,
    "model": false,
    "animation": false,
    "effect": false,
    "sound": false
  },
  "outputModel": "rawAIData",
  "engineHook": "applyAIMechanics"
}
)json");


    // ----------------------------
    // DROPS
    // ----------------------------
    writeFileIfMissing(coreDir, "drops.json", R"json(
{
  "id": "drops",
  "name": "Drop Tables",
  "scope": "core",
  "resourceFiles": [
    { "file": "propDropEvent.inc",   "domain": "drop" },
    { "file": "propGuildQuest.inc",  "domain": "drop" },
    { "file": "propFlyffpiece.inc",  "domain": "drop" }
  ],
  "parserRules": [
    { "pattern": "*.inc", "parserId": 0, "priority": 0 }
  ],
  "serializerKey": "drop",
  "contextBuilderKey": "drop",
  "dependencies": [ "items", "monster" ],
  "type": "drop",
  "assetsRequired": {
    "icon": false,
    "model": false,
    "animation": false,
    "effect": false,
    "sound": false
  },
  "outputModel": "rawDropData",
  "engineHook": ""
}
)json");


    // ----------------------------
    // QUESTS
    // ----------------------------
    writeFileIfMissing(coreDir, "quests.json", R"json(
{
  "id": "quests",
  "name": "Quests",
  "scope": "core",
  "resourceFiles": [
    { "file": "propQuest.inc",               "domain": "quest" },
    { "file": "propQuest-Scenario.inc",      "domain": "quest" },
    { "file": "propQuest-RequestBox.inc",    "domain": "quest" },
    { "file": "propQuest-DungeonandPK.inc",  "domain": "quest" },
    { "file": "propQuest-RequestBox2.inc",   "domain": "quest" }
  ],
  "parserRules": [
    { "pattern": "*.inc", "parserId": 0, "priority": 0 }
  ],
  "serializerKey": "quest",
  "contextBuilderKey": "quest",
  "dependencies": [ "text", "items", "monster", "jobs" ],
  "type": "quest",
  "assetsRequired": {
    "icon": false,
    "model": false,
    "animation": false,
    "effect": false,
    "sound": false
  },
  "outputModel": "rawQuestData",
  "engineHook": ""
}
)json");


    // ----------------------------
    // JOBS
    // ----------------------------
    writeFileIfMissing(coreDir, "jobs.json", R"json(
{
  "id": "jobs",
  "name": "Jobs / Classes",
  "scope": "core",
  "resourceFiles": [
    { "file": "propJob.inc", "domain": "job" }
  ],
  "parserRules": [
    { "pattern": "*.inc", "parserId": 0, "priority": 0 }
  ],
  "serializerKey": "job",
  "contextBuilderKey": "job",
  "dependencies": [ "defines", "skills" ],
  "type": "job",
  "assetsRequired": {
    "icon": false,
    "model": false,
    "animation": false,
    "effect": false,
    "sound": false
  },
  "outputModel": "rawJobData",
  "engineHook": ""
}
)json");


    // ----------------------------
    // LAYOUT
    // ----------------------------
    writeFileIfMissing(coreDir, "layout.json", R"json(
{
  "id": "layout",
  "name": "Windows / UI Layout",
  "scope": "core",
  "resourceFiles": [
    { "file": "resdata.inc", "domain": "ui" },
    { "file": "ResData.h",   "domain": "ui" }
  ],
  "parserRules": [
    { "pattern": "resdata.inc", "parserId": 1, "priority": 10 },
    { "pattern": "*.h",        "parserId": 0, "priority": 5 }
  ],
  "serializerKey": "layout",
  "contextBuilderKey": "layout",
  "dependencies": [ "text" ],
  "type": "layout",
  "assetsRequired": {
    "icon": false,
    "model": false,
    "animation": false,
    "effect": false,
    "sound": false
  },
  "outputModel": "rawLayoutData",
  "engineHook": "applyLayoutRules"
}
)json");


    // ----------------------------
    // ASSETS (mdlObj.inc + mdlDyna.inc)
    // ----------------------------
    writeFileIfMissing(coreDir, "assets.json", R"json(
{
  "id": "assets",
  "name": "Asset Model Definitions",
  "scope": "core",
  "resourceFiles": [
    { "file": "mdlObj.inc",  "domain": "asset" },
    { "file": "mdlDyna.inc", "domain": "asset" }
  ],
  "parserRules": [
    { "pattern": "*.inc", "parserId": 0, "priority": 10 }
  ],
  "serializerKey": "asset",
  "contextBuilderKey": "asset",
  "dependencies": [ "defines" ],
  "type": "asset",
  "assetsRequired": {
    "icon": false,
    "model": true,
    "animation": true,
    "effect": true,
    "sound": false
  },
  "outputModel": "rawAssetData",
  "engineHook": "registerModelAssets"
}
)json");

}

// ----------------------------
// LOAD
// ----------------------------
std::vector<Descriptor> DescriptorLoader::loadFromDirectory(const std::string& dir, const std::string& scope)
{
    std::vector<Descriptor> out;
    if (!fs::exists(dir)) return out;

    for (auto& e : fs::directory_iterator(dir))
    {
        if (!e.is_regular_file()) continue;
        if (e.path().extension() != ".json") continue;

        Descriptor d = loadFromFile(e.path().string(), scope);
        if (d.valid)
            out.push_back(std::move(d));
    }
    return out;
}

Descriptor DescriptorLoader::loadFromFile(
    const std::string& path,
    const std::string& scope
    )
{
    Descriptor d;

    std::ifstream file(path);
    if (!file.is_open())
        return d;

    json j;
    file >> j;

    d.id = j.value("id", "");
    d.name = j.value("name", "");
    d.scope = scope;

    d.serializerKey     = j.value("serializerKey", "");
    d.contextBuilderKey = j.value("contextBuilderKey", "");
    d.type              = j.value("type", "");
    d.outputModel       = j.value("outputModel", "");
    d.engineHook        = j.value("engineHook", "");

    if (j.contains("dependencies"))
    {
        for (auto& dep : j["dependencies"])
            d.dependencies.push_back(dep.get<std::string>());
    }

    if (j.contains("resourceFiles"))
    {
        for (const auto& rf : j["resourceFiles"])
        {
            ResourceFile entry;

            if (rf.is_string())
            {
                entry.file   = rf.get<std::string>();
                entry.domain = "";
            }
            else if (rf.is_object())
            {
                entry.file   = rf.value("file", "");
                entry.domain = rf.value("domain", "");
            }

            if (!entry.file.empty())
                d.resourceFiles.push_back(std::move(entry));
        }
    }

    d.valid = !d.id.empty();
    return d;
}
}
