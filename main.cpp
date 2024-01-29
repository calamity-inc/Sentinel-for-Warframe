#include "main.hpp"

#include <soup/format.hpp>
#include <soup/main.hpp>
#include <soup/time.hpp>

#include "data.hpp"
#include "DataCache.hpp"
#include "DuviriTarot.hpp"
#include "GeoIpService.hpp"
#include "Inventory.hpp"
#include "JsonHelper.hpp"
#include "LogDevotee.hpp"
#include "mission.hpp"
#include "Overlay.hpp"
#include "ProcessWatcher.hpp"
#include "squad.hpp" // local_name
#include "stdout.hpp"
#include "WindowCommons.hpp"
#include "WorldState.hpp"

static soup::Window w{};
static std::time_t log_timeout = 0;

using namespace Sentinel;

static constexpr uint8_t HEADING_SCALE = 2;
static constexpr unsigned int HEADING_PADDING_TOP = 10;
static constexpr unsigned int HEADING_PADDING_BOTTOM = 5;

static constexpr uint8_t TEXT_SCALE = 1;
static constexpr unsigned int TEXT_PADDING_BOTTOM = 2;

static void drawCountedItem(soup::RenderTarget& rt, unsigned int x, unsigned int y, const std::string& type, int count)
{
	std::string text = soup::format("- {}x {}", count, codename_to_english(type));
	int owned = Inventory::getOwnedCount(type);
	int crafted = 0;
	bool mastered = false;
	if (auto e = recipe_to_result_map.find(type); e != recipe_to_result_map.end())
	{
		crafted = Inventory::getOwnedCount(e->second);
		mastered = Inventory::isItemMastered(e->second);
	}
	else
	{
		mastered = Inventory::isItemMastered(type);
	}
	if (owned || crafted || mastered)
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
		if (mastered)
		{
			if (owned || crafted)
			{
				text.append(", ");
			}
			text.append("mastered");
		}
		text.push_back(')');
	}
	else
	{
		if (auto e = unique_ingredients_to_recipe_map.find(type); e != unique_ingredients_to_recipe_map.end())
		{
			if (e = recipe_to_result_map.find(e->second); e != recipe_to_result_map.end())
			{
				owned = Inventory::getOwnedCount(e->second);
				if (owned)
				{
					text.append(" (");
					text.append(std::to_string(owned));
					text.append("x ");
					text.append(codename_to_english(e->second));
					text.append(" owned)");
				}
				else
				{
					mastered = Inventory::isItemMastered(e->second);
					if (mastered)
					{
						text.append(" (");
						text.append(codename_to_english(e->second));
						text.append(" mastered)");
					}
				}
			}
		}
	}
	rt.drawText(x, y, std::move(text), soup::RasterFont::simple8(), owned || crafted || mastered ? soup::Rgb::GREY : soup::Rgb::WHITE, TEXT_SCALE);
}

static void drawHeading(soup::RenderTarget& rt, unsigned int x, unsigned int& y, const char* heading)
{
	y += HEADING_PADDING_TOP;
	rt.drawText(x, y, heading, soup::RasterFont::simple5(), soup::Rgb::WHITE, HEADING_SCALE);
	y += (HEADING_SCALE * 5) + HEADING_PADDING_BOTTOM;
}

static void drawItemsList(soup::RenderTarget& rt, unsigned int x, unsigned int& y, const char* heading, const std::vector<std::pair<std::string, int>>& items)
{
	if (!items.empty())
	{
		drawHeading(rt, x, y, heading);
		for (const auto& item : items)
		{
			drawCountedItem(rt, x, y, item.first, item.second);
			y += (TEXT_SCALE * 8) + TEXT_PADDING_BOTTOM;
		}
	}
}

[[nodiscard]] static const soup::JsonObject& getSyndicate(const std::string& tag)
{
	for (const auto& syndicate : WorldState::root->asObj().at("SyndicateMissions").asArr())
	{
		if (syndicate.asObj().at("Tag").asStr() == tag)
		{
			return syndicate.asObj();
		}
	}
	SOUP_ASSERT_UNREACHABLE;
}

[[nodiscard]] static std::vector<std::pair<std::string, int>> getInterestingBountyRewards(const soup::JsonObject& syndicate)
{
	std::vector<std::pair<std::string, int>> res{};
	for (const auto& job : syndicate.at("Jobs").asArr())
	{
		if (job.asObj().at("rewards").asStr() == "/Lotus/Types/Game/MissionDecks/EidolonJobMissionRewards/NarmerTableARewards")
		{
			res.emplace_back("/Lotus/Types/Recipes/WarframeRecipes/SentientSystemsBlueprint", 1);
			res.emplace_back("/Lotus/Types/Recipes/Weapons/NewWar/ArchonWhipBlueprint", 1);
		}
		else if (job.asObj().at("rewards").asStr() == "/Lotus/Types/Game/MissionDecks/EidolonJobMissionRewards/NarmerTableBRewards")
		{
			res.emplace_back("/Lotus/Types/Recipes/WarframeRecipes/SentientChassisBlueprint", 1);
			res.emplace_back("/Lotus/Types/Recipes/Weapons/NewWar/ArchonDualDaggersBlueprint", 1);
		}
		else if (job.asObj().at("rewards").asStr() == "/Lotus/Types/Game/MissionDecks/EidolonJobMissionRewards/NarmerTableCRewards")
		{
			res.emplace_back("/Lotus/Types/Recipes/WarframeRecipes/SentientHelmetBlueprint", 1);
			res.emplace_back("/Lotus/Types/Recipes/Weapons/NewWar/ArchonTridentBlueprint", 1);
		}
	}
	return res;
}

int entrypoint(std::vector<std::string>&& args, bool console)
{
	DataCache::init();
	DuviriTarot::readFromCache();
	if (ProcessWatcher::isGameRunning())
	{
		println("main", "Game is running, catching up with current session...");
		LogDevotee::process();
	}
	Inventory::load();

	WorldState::download();

	w = soup::Window::create("Sentinel for Warframe", 320, 380, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(101)));
	WindowCommons::init(w);
	w.setDrawFunc([](soup::Window w, soup::RenderTarget& rt)
	{
		rt.fill({ 0x10, 0x10, 0x10 });

		const unsigned int x = 4;
		unsigned int y = 4;

		rt.drawText(x, y, "Sentinel for Warframe", soup::RasterFont::simple8(), soup::Rgb::WHITE, 2);
		y += (2 * 8) + 4;

		if (Inventory::data_as_of)
		{
			rt.drawText(x, y, soup::format("Inventory data as of {}", soup::time::datetimeLocal(Inventory::data_as_of).toString()), soup::RasterFont::simple8(), soup::Rgb::WHITE, TEXT_SCALE);
		}
		else
		{
			rt.drawText(x, y, "Inventory data unavailable", soup::RasterFont::simple8(), soup::Rgb::WHITE, TEXT_SCALE);
		}
		y += (TEXT_SCALE * 8) + TEXT_PADDING_BOTTOM;

		if (WorldState::data_as_of)
		{
			rt.drawText(x, y, soup::format("World state as of {}", soup::time::datetimeLocal(WorldState::data_as_of).toString()), soup::RasterFont::simple8(), soup::Rgb::WHITE, TEXT_SCALE);
		}
		else
		{
			rt.drawText(x, y, "World state unavailable", soup::RasterFont::simple8(), soup::Rgb::WHITE, TEXT_SCALE);
		}
		y += (TEXT_SCALE * 8) + TEXT_PADDING_BOTTOM;

		if (WorldState::root)
		{
			if (Inventory::isLoaded())
			{
				drawHeading(rt, x, y, "PERIODIC MISSIONS");

				time_t week_began_at = 0;
				for (const auto& mission : WorldState::root->asObj().at("LiteSorties").asArr())
				{
					week_began_at = JsonHelper::readDate(mission.asObj().at("Activation"));
					break;
				}

				if (Inventory::getLastAyatanTreasureHuntCompletion() >= week_began_at)
				{
					rt.drawText(x, y, "- Ayatan Treasure Hunt (completed)", soup::RasterFont::simple8(), soup::Rgb::GREY, TEXT_SCALE);
				}
				else
				{
					rt.drawText(x, y, "- Ayatan Treasure Hunt", soup::RasterFont::simple8(), soup::Rgb::WHITE, TEXT_SCALE);
				}
				y += (TEXT_SCALE * 8) + TEXT_PADDING_BOTTOM;

				for (const auto& mission : WorldState::root->asObj().at("Sorties").asArr())
				{
					bool completed = Inventory::hasCompletedLatestSortie(mission.asObj().at("_id").asObj().at("$oid").asStr());
					if (completed
						|| just_completed_sortie // inventory data may not be instantly updated to reflect the completion
						)
					{
						rt.drawText(x, y, "- Sortie (completed)", soup::RasterFont::simple8(), soup::Rgb::GREY, TEXT_SCALE);
					}
					else
					{
						rt.drawText(x, y, "- Sortie", soup::RasterFont::simple8(), soup::Rgb::WHITE, TEXT_SCALE);
					}
					y += (TEXT_SCALE * 8) + TEXT_PADDING_BOTTOM;
					break;
				}

				for (const auto& mission : WorldState::root->asObj().at("LiteSorties").asArr())
				{
					bool completed = Inventory::hasCompletedLatestArchonHunt(mission.asObj().at("_id").asObj().at("$oid").asStr());
					if (completed)
					{
						rt.drawText(x, y, "- Archon Hunt (completed)", soup::RasterFont::simple8(), soup::Rgb::GREY, TEXT_SCALE);
					}
					else
					{
						rt.drawText(x, y, "- Archon Hunt", soup::RasterFont::simple8(), soup::Rgb::WHITE, TEXT_SCALE);
					}
					y += (TEXT_SCALE * 8) + TEXT_PADDING_BOTTOM;
					break;
				}

				int netracell_completions = Inventory::getNetracellCompletions();
				if (week_began_at >= Inventory::getNetracellCompletionsResetTime())
				{
					netracell_completions = 0;
				}
				if (netracell_completions == 5)
				{
					rt.drawText(x, y, "- Netracells (5/5 completed)", soup::RasterFont::simple8(), soup::Rgb::GREY, TEXT_SCALE);
				}
				else
				{
					rt.drawText(x, y, soup::format("- Netracells ({}/5 completed)", netracell_completions), soup::RasterFont::simple8(), soup::Rgb::WHITE, TEXT_SCALE);
				}
				y += (TEXT_SCALE * 8) + TEXT_PADDING_BOTTOM;
			}

			drawItemsList(rt, x, y, "BOUNTIES", getInterestingBountyRewards(getSyndicate("CetusSyndicate")));

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
			drawItemsList(rt, x, y, "INVASIONS", invasion_items);
		}

		if (LogDevotee::isGameRunning()
			&& current_missionType.empty()
			)
		{
			drawHeading(rt, x, y, "DUVIRI OPTIONS");
			std::lock_guard lock(duviri_items_mtx);
			for (const auto& item : duviri_items)
			{
				if (item.category == IC_POWERSUIT)
				{
					rt.drawText(x, y, soup::format("- Warframe: {}", codename_to_english(item.codename)), soup::RasterFont::simple8(), soup::Rgb::WHITE, 1);
					y += 10;
				}
			}
			for (const auto& item : duviri_items)
			{
				if (item.category == IC_PRIMARY)
				{
					rt.drawText(x, y, soup::format("- Primary Weapon: {}", codename_to_english(item.codename)), soup::RasterFont::simple8(), soup::Rgb::WHITE, 1);
					y += 10;
				}
			}
			for (const auto& item : duviri_items)
			{
				if (item.category == IC_SECONDARY)
				{
					rt.drawText(x, y, soup::format("- Secondary Weapon: {}", codename_to_english(item.codename)), soup::RasterFont::simple8(), soup::Rgb::WHITE, 1);
					y += 10;
				}
			}
			for (const auto& item : duviri_items)
			{
				if (item.category == IC_MELEE)
				{
					rt.drawText(x, y, soup::format("- Melee Weapon: {}", codename_to_english(item.codename)), soup::RasterFont::simple8(), soup::Rgb::WHITE, 1);
					y += 10;
				}
			}
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
	
	ProcessWatcher::init();
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
					println("main", "Game process doesn't exist anymore, but LogDevotee didn't deinit itself. Did you alt+f4 the game?!");
					LogDevotee::deinit();
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

	Overlay::deinit();

	t.join();

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
