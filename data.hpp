#pragma once

#include <string>
#include <unordered_map>

extern std::unordered_map<std::string, std::string> codename_to_english_map;
extern std::unordered_map<std::string, std::string> recipe_to_result_map;
extern std::unordered_map<std::string, std::string> unique_ingredients_to_recipe_map;
extern std::unordered_map<std::string, std::string> solnodes;

[[nodiscard]] inline std::string codename_to_english(const std::string& codename)
{
	if (auto e = codename_to_english_map.find(codename); e != codename_to_english_map.end())
	{
		return e->second;
	}
	if (auto e = recipe_to_result_map.find(codename); e != recipe_to_result_map.end())
	{
		std::string name = codename_to_english(e->second);
		name.append(" Blueprint");
		return name;
	}
	return codename;
}
