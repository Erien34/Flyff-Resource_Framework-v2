#include "SerializerBootstrap.h"
#include "SerializerRegistry.h"

// Core
#include "defines/DefineSerializer.h"
#include "text/TextSerializer.h"

// Gameplay
#include "gameplay/ItemSerializer.h"
#include "gameplay/SkillSerializer.h"
#include "gameplay/MonsterSerializer.h"
#include "gameplay/DropSerializer.h"
#include "gameplay/QuestSerializer.h"
#include "gameplay/JobSerializer.h"

// Other
#include "layout/LayoutSerializer.h"
#include "assets/AssetSerializer.h"
#include "ai/AISerializer.h"
#include "Log.h"

namespace modules::serializer
{
void registerCoreSerializers(SerializerRegistry& registry)
{
    registry.registerSerializer(std::make_unique<DefineSerializer>());
    registry.registerSerializer(std::make_unique<TextSerializer>());

    registry.registerSerializer(std::make_unique<ItemSerializer>());
    registry.registerSerializer(std::make_unique<SkillSerializer>());
    registry.registerSerializer(std::make_unique<MonsterSerializer>());
    registry.registerSerializer(std::make_unique<DropSerializer>());
    registry.registerSerializer(std::make_unique<QuestSerializer>());
    registry.registerSerializer(std::make_unique<JobSerializer>());

    registry.registerSerializer(std::make_unique<LayoutSerializer>());
    registry.registerSerializer(std::make_unique<AssetSerializer>());
    registry.registerSerializer(std::make_unique<AiSerializer>());

    core::Log::info("All Core-Serializer registered!");
}
}
