#include "DropSerializer.h"
#include "Log.h"

using namespace modules::serializer;

void DropSerializer::serialize(const std::vector<data::TokenData>& streams)
{
    m_data.streams = streams;
    m_data.valid = !streams.empty();
    core::Log::info("DropSerializer: collected " + std::to_string(streams.size()) + " streams");
}
