#include "GeoIpService.hpp"

#include "Overlay.hpp"
#include "squad.hpp"
#include "stdout.hpp"

namespace Sentinel
{
	static soup::netIntel net_intel{};

	void GeoIpService::init()
	{
		busy = true;
		std::thread t([]
		{
			println("GeoIpService", "Initialising netIntel for IPv4...");

			net_intel.init(true, false);
			available = true;
			println("GeoIpService", "netIntel for IPv4 is ready!");

			std::lock_guard lock(squad_members_mtx);
			if (!squad_members.empty())
			{
				Overlay::redraw();
			}

			busy = false;
		});
		t.detach();
	}

	soup::netIntel& GeoIpService::get()
	{
		return net_intel;
	}
}
