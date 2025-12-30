#include "AiSerializer.h"
#include "Log.h"

using namespace modules::serializer;

void AiSerializer::serialize(const std::vector<data::TokenData>& streams)
{
    m_data.streams = streams;
    m_data.valid = !streams.empty();
    core::Log::info("AiSerializer: collected " + std::to_string(streams.size()) + " streams");
}
