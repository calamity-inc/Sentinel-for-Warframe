#include "Inventory.hpp"

#include <filesystem>

#include <soup/aes.hpp>
#include <soup/format.hpp>
#include <soup/string.hpp>
#include <soup/time.hpp>

#include "data.hpp"
#include "JsonHelper.hpp"
#include "main.hpp"
#include "stdout.hpp"

#ifdef _DEBUG
#define WRITE_JSON_ON_READ true
#else
#define WRITE_JSON_ON_READ false
#endif

#if WRITE_JSON_ON_READ
#include <soup/FileWriter.hpp>
#endif

namespace Sentinel
{
	static const uint8_t key[16] = {
		76,
		69,
		79,
		45,
		65,
		76,
		69,
		67,
		9,
		69,
		79,
		45,
		65,
		76,
		69,
		67
	};

	static const uint8_t iv[16] = {
		49,
		50,
		70,
		71,
		66,
		51,
		54,
		45,
		76,
		69,
		51,
		45,
		113,
		61,
		57,
		0
	};

	void Inventory::load()
	{
		std::lock_guard lock(mtx);

		{
			std::wstring path = _wgetenv(L"localappdata");
			path.append(L"\\AlecaFrame\\lastData.dat");

			if (!std::filesystem::is_regular_file(path))
			{
				data_as_of = 0;
				root.reset();
				return;
			}

			data_as_of = soup::time::unixFromFile(std::filesystem::last_write_time(path));

			auto data = soup::string::fromFile(path);
			soup::aes::cbcDecrypt(
				reinterpret_cast<uint8_t*>(data.data()), data.size(),
				key, 16,
				iv
			);
			SOUP_ASSERT(soup::aes::pkcs7Unpad(data));
			root = soup::json::decode(data);
		}

#if WRITE_JSON_ON_READ
		{
			std::wstring path = _wgetenv(L"localappdata");
			path.append(L"\\AlecaFrame\\lastData.decrypted-by-sentinel.json");

			soup::FileWriter fw(path);
			auto pretty = root->encodePretty();
			fw.raw(pretty.data(), pretty.size());
		}
#endif

		if (root->asObj().contains("InventoryJson"))
		{
			root = soup::json::decode(root->asObj().at("InventoryJson").asStr());
		}

#if WRITE_JSON_ON_READ
		{
			std::wstring path = _wgetenv(L"localappdata");
			path.append(L"\\AlecaFrame\\lastData.tidied-by-sentinel.json");

			soup::FileWriter fw(path);
			auto pretty = root->encodePretty();
			fw.raw(pretty.data(), pretty.size());
		}
#endif

		println("Inventory", soup::format("Loaded data as of {}", soup::time::datetimeLocal(data_as_of).toString()));

		mainWindowRedraw();
	}

	int Inventory::getOwnedCount(const std::string& type)
	{
		std::lock_guard lock(mtx);
		if (root == nullptr)
		{
			return 0;
		}
		for (const auto& item : root->asObj().at("MiscItems").asArr())
		{
			if (item.asObj().at("ItemType").asStr() == type)
			{
				return item.asObj().at("ItemCount").asInt();
			}
		}
		for (const auto& item : root->asObj().at("Recipes").asArr())
		{
			if (item.asObj().at("ItemType").asStr() == type)
			{
				return item.asObj().at("ItemCount").asInt();
			}
		}
		int count = 0;
		for (const auto& item : root->asObj().at("LongGuns").asArr())
		{
			if (item.asObj().at("ItemType").asStr() == type)
			{
				++count;
			}
		}
		for (const auto& item : root->asObj().at("Melee").asArr())
		{
			if (item.asObj().at("ItemType").asStr() == type)
			{
				++count;
			}
		}
		for (const auto& item : root->asObj().at("Pistols").asArr())
		{
			if (item.asObj().at("ItemType").asStr() == type)
			{
				++count;
			}
		}
		for (const auto& item : root->asObj().at("Suits").asArr())
		{
			if (item.asObj().at("ItemType").asStr() == type)
			{
				++count;
			}
		}
		return count;
	}

	bool Inventory::getCraftedCount(const std::string& type)
	{
		if (auto e = recipe_to_result_map.find(type); e != recipe_to_result_map.end())
		{
			return getOwnedCount(e->second);
		}
		return 0;
	}

	int Inventory::getItemXp(const std::string& type)
	{
		std::lock_guard lock(mtx);
		if (root != nullptr)
		{
			for (const auto& item : root->asObj().at("XPInfo").asArr())
			{
				if (item.asObj().at("ItemType").asStr() == type)
				{
					return item.asObj().at("XP").asInt();
				}
			}
		}
		return 0;
	}

	int Inventory::isItemMastered(const std::string& type)
	{
		return getItemXp(type) >= 450'000; // 450.000 XP would be rank 30 for a weapon, so we're making a few assumptions here.
	}

	bool Inventory::hasCompletedLatestSortie(const std::string& oid)
	{
		std::lock_guard lock(mtx);
		if (root != nullptr)
		{
			if (auto e = root->asObj().find("LastSortieReward"))
			{
				for (const auto& mission : e->asArr())
				{
					if (mission.asObj().at("SortieId").asObj().at("$oid").asStr() == oid)
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	bool Inventory::hasCompletedLatestArchonHunt(const std::string& oid)
	{
		std::lock_guard lock(mtx);
		if (root != nullptr)
		{
			if (auto e = root->asObj().find("LastLiteSortieReward"))
			{
				for (const auto& mission : e->asArr())
				{
					if (mission.asObj().at("SortieId").asObj().at("$oid").asStr() == oid)
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	time_t Inventory::getLastAyatanTreasureHuntCompletion()
	{
		std::lock_guard lock(mtx);
		time_t latest = 0;
		if (root != nullptr)
		{
			if (auto e = root->asObj().find("PeriodicMissionCompletions"))
			{
				for (const auto& mission : e->asArr())
				{
					if (mission.asObj().at("tag").asStr().value.substr(0, 12) == "TreasureHunt")
					{
						time_t date = JsonHelper::readDate(mission.asObj().at("date"));
						if (date > latest)
						{
							latest = date;
						}
					}
				}
			}
		}
		return latest;
	}

	int Inventory::getNetracellCompletions()
	{
		std::lock_guard lock(mtx);
		if (root != nullptr)
		{
			if (auto e = root->asObj().find("EntratiVaultCountLastPeriod"))
			{
				return e->asInt();
			}
		}
		return 0;
	}

	time_t Inventory::getNetracellCompletionsResetTime()
	{
		std::lock_guard lock(mtx);
		if (root != nullptr)
		{
			if (auto e = root->asObj().find("EntratiVaultCountResetDate"))
			{
				return JsonHelper::readDate(*e);
			}
		}
		return 0;
	}
}
