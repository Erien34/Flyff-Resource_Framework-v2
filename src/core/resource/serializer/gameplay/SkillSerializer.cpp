#include "SkillSerializer.h"
#include "Log.h"

using namespace modules::serializer;

void SkillSerializer::serialize(const std::vector<data::TokenData>& streams)
{
    m_data.streams = streams;
    m_data.valid = !streams.empty();
    core::Log::info("QuestSerializer: collected " + std::to_string(streams.size()) + " streams");
}
