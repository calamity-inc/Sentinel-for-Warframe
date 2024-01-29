#include "WorldState.hpp"

#include <soup/DetachedScheduler.hpp>
#include <soup/HttpRequestTask.hpp>
#include <soup/time.hpp>

#include "main.hpp"
#include "stdout.hpp"

namespace Sentinel
{
	static soup::DetachedScheduler sched{};

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
				if (hrt.result.has_value())
				{
					if (WorldState::logging)
					{
						println("WorldState", "Download succeeded.");
						WorldState::logging = false;
					}
					WorldState::data_as_of = soup::time::unixSeconds();
					WorldState::root = soup::json::decode(hrt.result->body);
					mainWindowRedraw();
				}
				else
				{
					println("WorldState", "Download failed.");
					WorldState::logging = true;
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
		if (logging)
		{
			println("WorldState", "Beginning download.");
		}
	}

	std::string WorldState::getLastSortieMissionName()
	{
		if (root)
		{
			std::string nodeName{};
			for (const auto& variant : root->asObj().at("Sorties").asArr().at(0).asObj().at("Variants").asArr())
			{
				nodeName = variant.asObj().at("node").asStr();
			}
			nodeName.append("_Sortie");
			return nodeName;
		}
		return {};
	}
}
