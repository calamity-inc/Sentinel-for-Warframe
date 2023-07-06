#pragma once

#include <soup/netIntel.hpp>

namespace Sentinel
{
	struct GeoIpService
	{
		inline static bool available = false;
		inline static bool busy = false;

		static void init();
		[[nodiscard]] static soup::netIntel& get();
	};
}
