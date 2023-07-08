#include "main.hpp"

#include <soup/format.hpp>
#include <soup/main.hpp>
#include <soup/time.hpp>

#include "data.hpp"
#include "DataCache.hpp"
#include "DuviriTarot.hpp"
#include "GeoIpService.hpp"
#include "Inventory.hpp"
#include "LogDevotee.hpp"
#include "Overlay.hpp"
#include "ProcessWatcher.hpp"
#include "squad.hpp" // local_name
#include "stdout.hpp"
#include "WindowCommons.hpp"
#include "WorldState.hpp"

static soup::Window w{};
static std::time_t log_timeout = 0;

using namespace Sentinel;

static void drawCountedItem(soup::RenderTarget& rt, unsigned int x, unsigned int y, const std::string& type, int count)
{
	std::string text = soup::format("- {}x {}", count, codename_to_english(type));
	const auto owned = Inventory::getOwnedCount(type);
	const auto crafted = Inventory::getCraftedCount(type);
	if (owned || crafted)
	{
		text.append(" (");
		if (owned)
		{
			text.append(soup::format("{} owned", owned));
		}
		if (crafted)
		{
			if (owned)
			{
				text.append(", ");
			}
			text.append(soup::format("{} crafted", crafted));
		}
		text.push_back(')');
	}
	rt.drawText(x, y, std::move(text), soup::RasterFont::simple8(), owned || crafted ? soup::Rgb::GRAY : soup::Rgb::WHITE, 1);
}

static void drawItemsList(soup::RenderTarget& rt, unsigned int x, unsigned int& y, const char* heading, const std::vector<std::pair<std::string, int>>& items)
{
	if (!items.empty())
	{
		y += 10;
		rt.drawText(x, y, heading, soup::RasterFont::simple5(), soup::Rgb::WHITE, 2);
		y += 15;
		for (const auto& item : items)
		{
			drawCountedItem(rt, x, y, item.first, item.second);
			y += 10;
		}
	}
}

[[nodiscard]] static std::vector<std::pair<std::string, int>> getInterestingBountyRewardsFromSyndicate(const std::string& tag)
{
	std::vector<std::pair<std::string, int>> res{};
	for (const auto& syndicate : WorldState::root->asObj().at("SyndicateMissions").asArr())
	{
		if (syndicate.asObj().at("Tag").asStr() == tag)
		{
			for (const auto& job : syndicate.asObj().at("Jobs").asArr())
			{
				if (job.asObj().at("rewards").asStr() == "/Lotus/Types/Game/MissionDecks/EidolonJobMissionRewards/NarmerTableARewards")
				{
					res.emplace_back("/Lotus/Types/Recipes/WarframeRecipes/SentientSystemsBlueprint", 1);
				}
				else if (job.asObj().at("rewards").asStr() == "/Lotus/Types/Game/MissionDecks/EidolonJobMissionRewards/NarmerTableBRewards")
				{
					res.emplace_back("/Lotus/Types/Recipes/WarframeRecipes/SentientChassisBlueprint", 1);
				}
				else if (job.asObj().at("rewards").asStr() == "/Lotus/Types/Game/MissionDecks/EidolonJobMissionRewards/NarmerTableCRewards")
				{
					res.emplace_back("/Lotus/Types/Recipes/WarframeRecipes/SentientHelmetBlueprint", 1);
				}
			}
		}
	}
	return res;
}

int entrypoint(std::vector<std::string>&& args, bool console)
{
	println("main", "Welcome to Sentinel. First of all, I'm going to catch up with the most recent game session.");
	DataCache::init();
	DuviriTarot::readFromCache();
	LogDevotee::process();
	if (LogDevotee::isGameRunning())
	{
		Inventory::load();
		println("main", "All caught up!");
	}
	else
	{
		println("main", "Looks like the game is not running right now.");
		Inventory::load();
	}

	WorldState::download();

	w = soup::Window::create("Sentinel", 400, 200, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(101)));
	WindowCommons::init(w);
	w.setDrawFunc([](soup::Window w, soup::RenderTarget& rt)
	{
		rt.fill({ 0x10, 0x10, 0x10 });

		rt.drawText(2, 2, soup::format("Welcome to Sentinel, {}!", local_name.empty() ? "Tenno" : local_name), soup::RasterFont::simple8(), soup::Rgb::WHITE, 2);

		if (Inventory::data_as_of)
		{
			rt.drawText(2, 22, soup::format("Inventory data as of {}", soup::time::datetimeLocal(Inventory::data_as_of).toString()), soup::RasterFont::simple8(), soup::Rgb::WHITE);
		}
		else
		{
			rt.drawText(2, 22, "Inventory data unavailable", soup::RasterFont::simple8(), soup::Rgb::WHITE);
		}

		if (WorldState::data_as_of)
		{
			rt.drawText(2, 32, soup::format("World state as of {}", soup::time::datetimeLocal(WorldState::data_as_of).toString()), soup::RasterFont::simple8(), soup::Rgb::WHITE);
		}
		else
		{
			rt.drawText(2, 32, "World state unavailable", soup::RasterFont::simple8(), soup::Rgb::WHITE);
		}

		unsigned int y = 52 - 10;

		if (WorldState::root)
		{
			drawItemsList(rt, 2, y, "CETUS BOUNTIES", getInterestingBountyRewardsFromSyndicate("CetusSyndicate"));
			drawItemsList(rt, 2, y, "FORTUNA BOUNTIES", getInterestingBountyRewardsFromSyndicate("SolarisSyndicate"));

			std::vector<std::pair<std::string, int>> invasion_items{};
			for (const auto& invasion : WorldState::root->asObj().at("Invasions").asArr())
			{
				if (invasion.asObj().at("Completed").asBool())
				{
					continue;
				}
				if (invasion.asObj().at("AttackerReward").isObj())
				{
					for (const auto& countedItem : invasion.asObj().at("AttackerReward").asObj().at("countedItems").asArr())
					{
						invasion_items.emplace_back(
							countedItem.asObj().at("ItemType").asStr(),
							countedItem.asObj().at("ItemCount").asInt()
						);
					}
				}
				if (invasion.asObj().at("DefenderReward").isObj())
				{
					for (const auto& countedItem : invasion.asObj().at("DefenderReward").asObj().at("countedItems").asArr())
					{
						invasion_items.emplace_back(
							countedItem.asObj().at("ItemType").asStr(),
							countedItem.asObj().at("ItemCount").asInt()
						);
					}
				}
			}
			for (auto i = invasion_items.begin(); i != invasion_items.end(); )
			{
				if (i->first == "/Lotus/Types/Items/Research/EnergyComponent" // Fieldron
					|| i->first == "/Lotus/Types/Items/Research/ChemComponent" // Detonite Injector
					|| i->first == "/Lotus/Types/Items/Research/BioComponent" // Mutagen Mass
					|| i->first == "/Lotus/Types/Items/MiscItems/InfestedAladCoordinate" // Mutalist Alad V Nav Coordinate (does not seem to be included in inventory)
					)
				{
					i = invasion_items.erase(i);
				}
				else
				{
					++i;
				}
			}
			drawItemsList(rt, 2, y, "INVASIONS", invasion_items);
		}

		WindowCommons::get(w).draw(rt, { 0x10, 0x10, 0x10 });
	});
	w.setMouseInformer([](soup::Window w, unsigned int x, unsigned int y) -> soup::Window::on_click_t
	{
		if (auto f = WindowCommons::get(w).informMouse(w, x, y))
		{
			return f;
		}
		return nullptr;
	});
	
	std::thread t([]
	{
		while (w)
		{
			const bool game_running = ProcessWatcher::isGameRunning();

			if (game_running)
			{
				if (!GeoIpService::available && !GeoIpService::busy)
				{
					GeoIpService::init();
				}
			}

			if (game_running)
			{
				if (soup::time::millisSince(log_timeout) > 10000)
				{
					LogDevotee::process();
					if (!LogDevotee::isGameRunning())
					{
						// LogDevotee thinks game is no longer running.
						// ProcessWatcher may lag behind, so we'll just temporarily stop LogDevotee processing.
						log_timeout = soup::time::millis();
					}
				}
			}
			else
			{
				if (LogDevotee::isGameRunning())
				{
					LogDevotee::process();
				}
			}

			if (game_running)
			{
				if (!Overlay::isInited())
				{
					if (ProcessWatcher::isGameFocused())
					{
						Overlay::init();
					}
				}
			}
			else
			{
				if (Overlay::isInited())
				{
					Overlay::deinit();
				}
			}

			if (soup::time::unixSecondsSince(WorldState::data_as_of) > 60)
			{
				if (!WorldState::request_in_flight)
				{
					WorldState::download();
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
	});

	w.runMessageLoop();
	w.reset();

	t.join();
	Overlay::deinit();

	if (GeoIpService::busy)
	{
		println("main", "Ready for shutdown, but GeoIpService is still busy. Please wait.");
		do
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(30));
		} while (GeoIpService::busy);
		println("main", "GeoIpService is idle now, proceeding with shutdown.");
	}

	if (WorldState::request_in_flight)
	{
		println("main", "Ready for shutdown, but WorldState is still busy. Please wait.");
		do
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(30));
		} while (WorldState::request_in_flight);
		println("main", "WorldState is idle now, proceeding with shutdown.");
	}

	return 0;
}

void mainWindowRedraw()
{
	if (w)
	{
		w.redraw();
	}
}

#ifdef _DEBUG
SOUP_MAIN_CLI(entrypoint);
#else
SOUP_MAIN_GUI(entrypoint);
#endif
