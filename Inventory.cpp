#include "Inventory.hpp"

#include <filesystem>

#include <soup/aes.hpp>
#include <soup/format.hpp>
#include <soup/joaat.hpp>
#include <soup/string.hpp>
#include <soup/time.hpp>

#include "data.hpp"
#include "main.hpp"
#include "stdout.hpp"

#define WRITE_JSON_ON_READ false

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

			auto decrypted = soup::aes::decryptCBC(
				soup::string::fromFilePath(path),
				std::string((const char*)key, 16),
				std::string((const char*)iv, 16)
			);
			soup::aes::pkcs7Unpad(decrypted);
			root = soup::json::decode(decrypted);
		}

#if WRITE_JSON_ON_READ
		{
			std::wstring path = _wgetenv(L"localappdata");
			path.append(L"\\AlecaFrame\\lastData.decrypted-by-sentinel.json");

			soup::FileWriter fw(path);
			auto pretty = root->encodePretty();
			fw.write(pretty.data(), pretty.size());
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
			fw.write(pretty.data(), pretty.size());
		}
#endif

		println("Inventory", soup::format("Loaded data as of {}", soup::time::datetimeLocal(data_as_of).toString()));

		mainWindowRedraw();
	}

	int Inventory::getOwnedCount(const std::string& type)
	{
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
}
