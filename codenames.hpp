#pragma once

#include <string>
#include <unordered_map>

extern std::unordered_map<std::string, std::string> codename_to_english_map;

inline std::string codename_to_english(const std::string& codename)
{
	if (auto e = codename_to_english_map.find(codename); e != codename_to_english_map.end())
	{
		return e->second;
	}
	return codename;
}
