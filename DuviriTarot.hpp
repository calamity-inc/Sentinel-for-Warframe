#pragma once

#include <mutex>
#include <string>
#include <vector>

enum ItemCategory
{
	IC_SECONDARY = 0,
	IC_PRIMARY = 1,
	IC_POWERSUIT = 3,
	IC_MELEE = 5,
};

struct Item
{
	std::string codename;
	int category;
};

inline std::recursive_mutex duviri_items_mtx{};
inline std::vector<Item> duviri_items{};

namespace Sentinel
{
	struct DuviriTarot
	{
		static void readFromCache();
		static void writeToCache();
	};
}
