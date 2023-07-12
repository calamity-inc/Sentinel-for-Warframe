#include "LogDevotee.hpp"

#include <soup/string.hpp>

#include "DuviriTarot.hpp"
#include "Inventory.hpp"
#include "main.hpp"
#include "mission.hpp"
#include "Overlay.hpp"
#include "squad.hpp"
#include "stdout.hpp"

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
		char buf[4096];
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
					std::string data;
					data += msg.substr(16);
					auto arr = string::explode(data, ", ");
					SquadMember member;
					member.name = arr.at(0);
					if (member.getName() != local_name)
					{
						member.mm = arr.at(1).substr(3); // "mm="
						// arr.at(2) == "squadCount=1"
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
					member_name.erase(member_name.find(' '));
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
					if (auto member = squadMemberByMM(data.substr(0, 24)))
					{
						data.erase(0, 26); // mm + " ("
						data.pop_back(); // '\r'
						data.pop_back(); // '\n'
						data.pop_back(); // ')'
						auto sep = data.find(':');
						member->ip = data.substr(0, sep);
						member->port = data.substr(sep + 1);
						std::cout << "[LogDevotee] Got IP for " << member->getName() << ": " << member->ip << "\n";
						Overlay::redraw();
					}
					else
					{
						std::cout << "[LogDevotee] Did not find a member with mm=" << data.substr(0, 24) << "\n";
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
}
