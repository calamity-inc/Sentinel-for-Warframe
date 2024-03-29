#include "LogDevotee.hpp"

#include <soup/IpAddr.hpp>
#include <soup/string.hpp>

#include "cidr.hpp"
#include "DuviriTarot.hpp"
#include "Inventory.hpp"
#include "main.hpp"
#include "mission.hpp"
#include "Overlay.hpp"
#include "squad.hpp"
#include "stdout.hpp"
#include "WorldState.hpp"

namespace Sentinel
{
	bool LogDevotee::isGameRunning()
	{
		return f != NULL;
	}

	void LogDevotee::deinit()
	{
		if (f != NULL)
		{
			fclose(f);
			f = NULL;
		}
	}

	// Variant of fgets that only succeeds if the line is terminated with '\n' to avoid reading incomplete lines.
	[[nodiscard]] static char* fgets_complete(char* buffer, int size, FILE* file)
	{
		long initial_position = ftell(file);
		if (fgets(buffer, size, file) == NULL)
		{
			return NULL;
		}
		if (strchr(buffer, '\n') == NULL)
		{
			fseek(file, initial_position, SEEK_SET);
			return NULL;
		}
		return buffer;
	}

	[[nodiscard]] static std::pair<std::string, std::string> parse_address(const std::string& data)
	{
		auto sep = data.find(':');
		auto ip = data.substr(0, sep);
		auto port = data.substr(sep + 1);
		return { std::move(ip), std::move(port) };
	}

	static std::string punchthrough_failure_addr{};

	void LogDevotee::process()
	{
		using namespace soup;

		if (f == NULL)
		{
			std::wstring path = _wgetenv(L"localappdata");
			path.append(L"\\Warframe\\EE.log");
			f = _wfopen(path.c_str(), L"rb");
			if (f == NULL)
			{
				return;
			}
		}
		char buf[0x2000];
		output_mutex.lock();
		while (fgets_complete(buf, sizeof(buf), f) != nullptr)
		{
			std::string_view sv(buf);
			if (auto i = sv.find("]: "); i != std::string::npos)
			{
				auto msg = sv.substr(i + 3);
				if (msg.substr(0, 10) == "Logged in ")
				{
					std::string name;
					name += msg.substr(10);
					name.erase(name.find(' '));
					local_name = std::move(name);
					host_name = local_name;
					std::cout << "[LogDevotee] Hello there, " << local_name << "!\n";
					// could also get account id from this if we needed it
				}
				else if (msg == "MatchingService::HostSquadSession\r\n")
				{
					std::lock_guard lock(squad_members_mtx);
					if (host_name != local_name
						|| !squad_members.empty()
						)
					{
						std::cout << "[LogDevotee] Creating a new squad even tho we didn't leave the previous squad?!\n";
						host_name = local_name;
						squad_members.clear();
					}
				}
				else if (msg.substr(0, 16) == "AddSquadMember: ")
				{
					SquadMember member;
					auto name_end = msg.find(", mm=", 16);
					member.name = msg.substr(16, name_end - 16);
					if (member.getName() != local_name)
					{
						auto mm_end = msg.find(", squadCount=", name_end);
						member.mm = msg.substr(name_end + 5, mm_end - (name_end + 5));
						std::cout << "[LogDevotee] New squad member: " << member.getName() << "\n";
						std::lock_guard lock(squad_members_mtx);
						squad_members.emplace_back(std::move(member));
						Overlay::redraw();
					}
				}
				else if (msg.substr(0, 19) == "RemoveSquadMember: ")
				{
					std::string member_name;
					member_name += msg.substr(19);
					member_name.erase(msg.find(" has been removed from the squad") - 19);
					std::lock_guard lock(squad_members_mtx);
					if (auto m = squadMemberByName(member_name); m != squad_members.end())
					{
						std::cout << "[LogDevotee] " << m->getName() << " has left our squad.\n";
						squad_members.erase(m);
						Overlay::redraw();
					}
				}
				else if (msg == "Sending LEAVE message\r\n"
					// || msg == "ThemedSquadOverlay.lua: Squad overlay - _LeaveSquad\r\n"
					)
				{
					// We have left the squad.
					host_name = local_name;
					std::lock_guard lock(squad_members_mtx);
					squad_members.clear();
					Overlay::redraw();
				}
				else if (msg.substr(0, 31) == "VOIP: Registered remote player ")
				{
					std::string data;
					data += msg.substr(31);
					auto sep = data.find(" (");
					if (auto member = squadMemberByMM(data.substr(0, sep)))
					{
						data.erase(0, sep + 2); // mm + " ("
						data.pop_back(); // '\r'
						data.pop_back(); // '\n'
						data.pop_back(); // ')'
						auto [ip, port] = parse_address(data);
						soup::IpAddr ipaddr;
						SOUP_ASSERT(ipaddr.fromString(ip));
						if (member->ip.empty() || !akamai.contains(ipaddr)) // Don't use an Akamai IP unless it's the first thing we're getting.
						{
							std::cout << "[LogDevotee] Got IP for " << member->getName() << ": " << ip << "\n";
							member->ip = std::move(ip);
							member->port = std::move(port);
						}
						Overlay::redraw();
					}
					else
					{
						std::cout << "[LogDevotee] Did not find a member with mm=" << data.substr(0, sep) << "\n";
					}
				}
				else if (msg.substr(0, 27) == "Failed to punch-through to ")
				{
					std::string data;
					data += msg.substr(53);
					data.pop_back(); // '\r'
					data.pop_back(); // '\n'
					data.pop_back(); // ')'
					punchthrough_failure_addr = std::move(data);
				}
				else if (msg.substr(0, 32) == "VOIP: punch-through failure for ")
				{
					std::string mm;
					mm += msg.substr(32, msg.size() - 32 - 2);
					if (auto member = squadMemberByMM(mm))
					{
						if (!punchthrough_failure_addr.empty())
						{
							auto [ip, port] = parse_address(punchthrough_failure_addr);
							punchthrough_failure_addr.clear();
							std::cout << "[LogDevotee] Got IP for " << member->getName() << ": " << ip << "\n";
							member->ip = std::move(ip);
							member->port = std::move(port);
						}
						else
						{
							std::cout << "[LogDevotee] VOIP punch-through failure but no address to go along with it\n";
						}
					}
					else
					{
						std::cout << "[LogDevotee] Did not find a member with mm=" << mm << "\n";
					}
				}
				else if (msg.substr(0, 24) == "JoinSquadSessionCallback")
				{
					host_name.clear();
					host_name += msg.substr(msg.find("host name=") + 10);
					host_name.erase(host_name.size() - 5); // platform indicator + "\r\n"
				}
				else if (msg.substr(0, 37) == "PrepareDemoCinematics.lua: Host name ")
				{
					host_name.clear();
					host_name += msg.substr(37);
					host_name.erase(host_name.size() - 5); // platform indicator + "\r\n"
				}
				else if (msg.substr(0, 29) == "HOST MIGRATION: local client " // ... selected as the new host
					|| msg == "Host migration: local client selected as the new host\r\n"
					)
				{
					host_name = local_name;
					Overlay::redraw();
				}
				else if (msg.substr(0, 54) == "HOST MIGRATION: local client trying to join new host: ")
				{
					host_name.clear();
					host_name += msg.substr(54);
					host_name.erase(host_name.size() - 5); // platform indicator + "\r\n"
				}
				else if (msg.substr(0, 37) == "ThemedSquadOverlay.lua: Host loading ")
				{
					std::string missionJson{};
					missionJson += msg.substr(37, msg.size() - 37 - 21); // " with MissionInfo: \r\n"
					processMissionJson(missionJson);
				}
				else if (msg.substr(0, 14) == "Client loaded ")
				{
					std::string missionJson{};
					missionJson += msg.substr(14, msg.size() - 14 - 20); // " with MissionInfo:\r\n"
					processMissionJson(missionJson);
				}
				else if (msg.substr(0, 29) == "OnStateStarted, mission type=")
				{
					current_missionType.clear();
					current_missionType += msg.substr(29, msg.size() - 29 - 2);
					mission_stage = 0;
					hostage_cell = -1;
					mainWindowRedraw();
					if (amHost())
					{
						Overlay::redraw();
					}
					std::cout << "[LogDevotee] Starting " << current_missionType << " mission\n";
				}
				else if (msg.substr(0, 28) == "EOM missionLocationUnlocked=")
				{
					current_missionType.clear();
					if (current_missionName == WorldState::getLastSortieMissionName())
					{
						just_completed_sortie = true;
					}
					mainWindowRedraw();
					if (amHost())
					{
						Overlay::redraw();
					}
					std::cout << "[LogDevotee] Mission over\n";
				}
				else if (msg.substr(0, 25) == "Rescue.lua: Hostage cell=")
				{
					hostage_cell = (msg.at(25) - '0');
					if (!amHost())
					{
						std::cout << "[LogDevotee] We're starting a mission as the host, but " << host_name << " should be the host?!\n";
						host_name = local_name;
					}
					std::cout << "[LogDevotee] Rescue target is in cell " << hostage_cell << "\n";
					if (amHost())
					{
						Overlay::redraw();
					}
				}
				else if (msg == "HudRedux.lua: Queuing new transmission: RescueEnterObjectiveRoomTransmission\r\n"
					|| msg == "HudRedux.lua: Queuing new transmission: RescueMissionSearchCellsTransmission\r\n"
					|| msg == "HudRedux.lua: Queuing new transmission: DRscInfApproachHold130LotusTransmission\r\n" // /Lotus/Levels/Proc/Infestation/InfestedCorpusShipRescue
					|| msg == "HudRedux.lua: Queuing new transmission: DRscMnUseConsolesLotus\r\n" // /Lotus/Levels/Proc/Orokin/OrokinMoonRescue
					)
				{
					// This is quite delayed on Lua.
					if (mission_stage != 1)
					{
						std::cout << "[LogDevotee] Entered holding area.\n";
						mission_stage = 1;
						/*if (amHost())
						{
							Overlay::redraw();
						}*/
					}
				}
				else if (msg == "HudRedux.lua: Queuing new transmission: ObjectiveFoundRescueTransmission\r\n"
					|| msg == "HudRedux.lua: Queuing new transmission: DRscInfObjFnd090LotusTransmission\r\n" // /Lotus/Levels/Proc/Infestation/InfestedCorpusShipRescue
					|| msg == "HudRedux.lua: Queuing new transmission: DRscMnAgentFoundLotus\r\n" // /Lotus/Levels/Proc/Orokin/OrokinMoonRescue
					)
				{
					std::cout << "[LogDevotee] Hostage found.\n";
					mission_stage = 2;
					if (amHost())
					{
						Overlay::redraw();
					}
				}
				else if (msg.substr(0, 14) == "MapRedux.lua: ")
				{
					if (msg == "MapRedux.lua: RollDuviriOffers - Generating new Duviri offers\r\n")
					{
						std::lock_guard lock(duviri_items_mtx);
						duviri_items.clear();
					}
					else if (auto i = msg.find(" generated as an ", 14); i != std::string::npos)
					{
						Item item;
						item.codename += msg.substr(14, i - 14);
						item.category = (msg.at(msg.find(" Cave selection for category: ", i) + 30) - '0');
						std::lock_guard lock(duviri_items_mtx);
						bool found = false;
						for (const auto& i : duviri_items)
						{
							if (i.codename == item.codename)
							{
								found = true;
								break;
							}
						}
						if (!found)
						{
							duviri_items.emplace_back(std::move(item));
						}
					}
					else if (msg == "MapRedux.lua: Found 0 overrides.\r\n")
					{
						// This should mean duviri_items are now finished generating.
						mainWindowRedraw();

						// The game will only generate them once per mood, so Sentinel needs to remember in case the game is restarted.
						DuviriTarot::writeToCache();
					}
				}
				else if (msg.substr(0, 32) == "OnInventoryResults completed in ")
				{
					if (Inventory::isLoaded())
					{
						std::cout << "[LogDevotee] Game has just downloaded inventory data, looks like a good time to reload it.\n";
						Inventory::load();
					}
				}
				/*else if (msg == "GameRulesImpl::EndSessionCallback\r\n")
				{
					if (Inventory::isLoaded())
					{
						std::cout << "[LogDevotee] A game session ended, looks like a good time to reload inventory data.\n";
						Inventory::load();
					}
				}*/
				else if (msg == "Main Shutdown Complete.\r\n")
				{
					std::cout << "[LogDevotee] Game closed.\n";
					fclose(f);
					f = NULL;
					output_mutex.unlock();
					return;
				}
			}
			else
			{
				if (sv.substr(0, 18) == R"(    levelOverride=)")
				{
					// When we're the host: ThemedSquadOverlay.lua: Host loading {...} with MissionInfo:
					if (!amHost())
					{
						std::cout << "[LogDevotee] We're starting a mission as the host, but " << host_name << " should be the host?!\n";
						host_name = local_name;
					}
					current_levelOverride.clear();
					current_levelOverride += sv.substr(18, sv.size() - 18 - 2);
				}
				else if (sv.substr(0, 23) == R"(    "levelOverride" : ")")
				{
					// When we're the client: Client loaded {"difficulty":"","voidTier":"VoidT1","name":"SolNode109_ActiveMission","quest":""} with MissionInfo:
					if (amHost())
					{
						std::cout << "[LogDevotee] We're starting a mission as a client, but we should be the host?!\n";
					}
					current_levelOverride.clear();
					current_levelOverride += sv.substr(23, sv.size() - 23 - 4);
				}
			}
		}
		output_mutex.unlock();
	}

	void LogDevotee::processMissionJson(const std::string& missionJson)
	{
		// Void Fissure: {"difficulty":"","voidTier":"VoidT4","name":"SolNode162_ActiveMission","quest":""}
		// Kuva Lich Controlled: {"difficulty":0.25,"name":"SolNode65_Nemesis","nemesis":{"faction":0,"name":"Demu Udoff","rank":1}}

		current_missionName.clear();
		just_completed_sortie = false;

		if (!missionJson.empty()) // apparently it's empty for Ayatan Treasure Hunt
		{
			auto root = soup::json::decode(missionJson);
			SOUP_ASSERT(root);
			if (root->asObj().contains("name")) // bounties sometimes only have "jobLevelGenerationSeed"
			{
				current_missionName = root->asObj().at("name").asStr();
			}
		}
	}
}
