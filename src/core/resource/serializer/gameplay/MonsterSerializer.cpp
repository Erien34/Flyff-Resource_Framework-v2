#include "MonsterSerializer.h"
#include "Log.h"

using namespace modules::serializer;

void MonsterSerializer::serialize(const std::vector<data::TokenData>& streams)
{
    m_data.streams = streams;
    m_data.valid = !streams.empty();
    core::Log::info("MonsterSerializer: collected " + std::to_string(streams.size()) + " streams");
	publishModel(outputModel(), m_data);
}
