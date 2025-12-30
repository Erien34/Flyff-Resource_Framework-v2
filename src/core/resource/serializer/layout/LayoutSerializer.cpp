#include "LayoutSerializer.h"
#include "Log.h"

using namespace modules::serializer;

void LayoutSerializer::serialize(const std::vector<data::TokenData>& streams)
{
    m_data.streams = streams;
    m_data.valid = !streams.empty();
    core::Log::info("LayoutSerializer: collected " + std::to_string(streams.size()) + " streams");
}
