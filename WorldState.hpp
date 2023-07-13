#pragma once

#include <soup/json.hpp>

namespace Sentinel
{
	struct WorldState
	{
		inline static bool logging = true;
		inline static bool request_in_flight = false;
		inline static time_t data_as_of = 0;
		inline static soup::UniquePtr<soup::JsonNode> root;

		static void download();
		
		[[nodiscard]] static std::string getLastSortieMissionName();
	};
}
