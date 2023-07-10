#pragma once

#include <mutex>
#include <string>
#include <vector>

struct SquadMember
{
	std::string name;
	std::string mm;
	// mm value of Windows client looks like this: "AB0D55C3C7B9CD00F768C265"
	// For cross-platform clients, mm value is either their username or looks like this: "S]F;JEV7B5';K%&           @8P(7=P-'- $          "
	//                                                                                   "OQ69SE78EQF9              08X(7=P-'- $          "
	// IP address & port are only available for Windows clients because there's no cross-platform VOIP.
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
