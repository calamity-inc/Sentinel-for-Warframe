#include "WorldState.hpp"

#include <soup/DetachedScheduler.hpp>
#include <soup/HttpRequestTask.hpp>
#include <soup/time.hpp>

#include "main.hpp"
#include "stdout.hpp"

namespace Sentinel
{
	inline soup::DetachedScheduler sched{};

	struct WorldStateDownloadTask : public soup::Task
	{
		soup::HttpRequestTask hrt;

		WorldStateDownloadTask()
			: hrt(soup::HttpRequest("content.warframe.com", "/dynamic/worldState.php"))
		{
		}

		void onTick() final
		{
			if (hrt.tickUntilDone())
			{
				if (hrt.res.has_value())
				{
					println("WorldState", "Download succeeded.");
					WorldState::data_as_of = soup::time::unixSeconds();
					WorldState::root = soup::json::decode(hrt.res->body);
					mainWindowRedraw();
				}
				else
				{
					println("WorldState", "Download failed.");
				}
				WorldState::request_in_flight = false;
				setWorkDone();
			}
		}
	};

	void WorldState::download()
	{
		SOUP_ASSERT(!request_in_flight);
		request_in_flight = true;
		sched.add<WorldStateDownloadTask>();
		println("WorldState", "Beginning download.");
	}
}
