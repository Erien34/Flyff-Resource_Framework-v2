#include "QuestSerializer.h"
#include "Log.h"

using namespace modules::serializer;

void QuestSerializer::serialize(const std::vector<data::TokenData>& streams)
{
    m_data.streams = streams;
    m_data.valid = !streams.empty();
    core::Log::info("QuestSerializer: collected " + std::to_string(streams.size()) + " streams");
}
