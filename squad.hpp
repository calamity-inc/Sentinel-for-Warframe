#pragma once

#include <mutex>
#include <string>
#include <vector>

// For windows clients, the username is suffixed with U+E000 ().
// Clients of other platforms have the U+2068 () suffix.

// The mm value of playstation clients has been confirmed via chat.
// Xbox and Switch may be the other way around, but guessing Xbox is more popular.

// IP address & port are only available for Windows clients because there's no cross-platform VOIP.

enum Platform
{
	WINDOWS, // mm value looks like this: "AB0D55C3C7B9CD00F768C265"
	PLAYSTATION, // mm value is their username (without the U+2068 suffix)
	XBOX,   // mm value looks like this: "S]F;JEV7B5';K%&           @8P(7=P-'- $          "
	        //                           "OQ69SE78EQF9              08X(7=P-'- $          "
	SWITCH, // mm value looks like this: "2535409713692376"
};

[[nodiscard]] inline const char* platformToString(Platform p)
{
	switch (p)
	{
	case WINDOWS: return "Windows";
	case PLAYSTATION: return "PlayStation";
	case XBOX: return "Xbox";
	case SWITCH: return "Nintendo Switch";
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
			return PLAYSTATION;
		}
		if (mm.size() > 32)
		{
			return XBOX;
		}
		return SWITCH;
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
