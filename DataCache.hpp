#pragma once

#include <soup/json.hpp>

namespace Sentinel
{
	struct DataCache
	{
		inline static soup::UniquePtr<soup::JsonNode> root;

		static void init();
		static void commit();
	};
}
