#pragma once

#include <soup/json.hpp>

namespace Sentinel
{
	// For warframes and sentinels
	[[nodiscard]] inline int getXpRequiredForRank(int rank)
	{
		return rank * rank * 1000;
	}

	// For weapons
	[[nodiscard]] inline int getXpRequiredForWeaponRank(int rank)
	{
		return getXpRequiredForRank(rank) / 2;
	}

	// For warframes and sentinels
	[[nodiscard]] inline int getRankFromXp(int xp)
	{
		return (int)floorf(sqrtf(xp / 1000.0f));
	}

	// For weapons
	[[nodiscard]] inline int getWeaponRankFromXp(int xp)
	{
		return getRankFromXp(xp * 2);
	}

	struct Inventory
	{
		inline static time_t data_as_of = 0;
		inline static soup::UniquePtr<soup::JsonNode> root;
		
		[[nodiscard]] static bool isLoaded() noexcept
		{
			return data_as_of != 0;
		}

		static void load();

		[[nodiscard]] static int getOwnedCount(const std::string& type);
		[[nodiscard]] static bool getCraftedCount(const std::string& type);
		[[nodiscard]] static int getItemXp(const std::string& type);
		[[nodiscard]] static int isItemMastered(const std::string& type);

		[[nodiscard]] static bool hasCompletedLatestSortie(const std::string& oid);
		[[nodiscard]] static bool hasCompletedLatestArchonHunt(const std::string& oid);
	};
}
