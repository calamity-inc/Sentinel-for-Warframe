#pragma once

#include <mutex>
#include <string>
#include <vector>

// For windows clients, the username is suffixed with U+E000 ().
// Clients of other platforms have the U+2068 () suffix.

// IP address & port are only available for Windows clients because there's no cross-platform VOIP.

enum Platform
{
	WINDOWS, // mm value looks like this: "AB0D55C3C7B9CD00F768C265"
	PLAYSTATION_5, // mm value is their username (without the U+2068 suffix)
	PLAYSTATION_4, // mm value looks like this: "'%6;IYV9/5'=SE&9EA'       @8R069P-'- $          "
	XBOX,   // mm value looks like this: "2535409713692376"
	SWITCH, // mm value looks like "8087900890037930173" or "17479101469666850951"
	UNKNOWN
};

[[nodiscard]] inline const char* platformToString(Platform p)
{
	switch (p)
	{
	case WINDOWS: return "Windows";
	case PLAYSTATION_5: return "PlayStation 5";
	case PLAYSTATION_4: return "PlayStation 4";
	case XBOX: return "Xbox";
	case SWITCH: return "Nintendo Switch";
	case UNKNOWN: return "Unknown";
	}
	SOUP_ASSERT_UNREACHABLE;
}

struct SquadMember
{
	std::string name;
	std::string mm;
	std::string ip;
	std::string port;

	[[nodiscard]] std::string getName() const
	{
		// excluding platform symbol
		return name.substr(0, name.size() - 3);
	}

	[[nodiscard]] bool isWindows() const
	{
		return name.substr(name.size() - 3) == (const char*)u8"\uE000";
	}

	[[nodiscard]] Platform getPlatform() const
	{
		if (isWindows())
		{
			return WINDOWS;
		}
		if (getName() == mm)
		{
			return PLAYSTATION_5;
		}
		if (mm.size() == 48)
		{
			return PLAYSTATION_4;
		}
		if (mm.size() == 16)
		{
			return XBOX;
		}
		if (mm.size() == 19 || mm.size() == 20)
		{
			return SWITCH;
		}
		return UNKNOWN;
	}
};

inline std::string local_name;
inline std::string host_name;
inline std::recursive_mutex squad_members_mtx{};
inline std::vector<SquadMember> squad_members{};

[[nodiscard]] inline bool amHost() noexcept
{
	return local_name == host_name;
}

[[nodiscard]] inline std::vector<SquadMember>::iterator squadMemberByName(const std::string& name)
{
	std::lock_guard lock(squad_members_mtx);
	auto i = squad_members.begin();
	for (; i != squad_members.end(); ++i)
	{
		if (i->name == name)
		{
			break;
		}
	}
	return i;
}

[[nodiscard]] inline SquadMember* squadMemberByMM(const std::string& mm)
{
	std::lock_guard lock(squad_members_mtx);
	for (auto& squad_member : squad_members)
	{
		if (squad_member.mm == mm)
		{
			return &squad_member;
		}
	}
	return nullptr;
}
