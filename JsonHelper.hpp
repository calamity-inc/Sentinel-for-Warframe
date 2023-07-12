#pragma once

#include <soup/json.hpp>

namespace Sentinel
{
	struct JsonHelper
	{
		[[nodiscard]] static std::time_t readDate(const soup::JsonNode& obj)
		{
			return std::stoull(obj.asObj().at("$date").asObj().at("$numberLong").asStr()) / 1000;
		}
	};
}
