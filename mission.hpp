#pragma once

#include <string>

inline std::string current_missionType;
inline std::string current_levelOverride;
inline int hostage_cell = -1;
inline int mission_stage;

[[nodiscard]] inline const char* getCellName(const std::string& tileset, int cell)
{
	if (tileset == "/Lotus/Levels/Proc/Grineer/GrineerGalleonRescue") // Earth & Sedna
	{
		// /Lotus/Levels/GrineerGalleon/GrnRescueObjectiveRoom.level, 6 cells
		switch (cell)
		{
		case 1: return "Top, right, front."; // Prefab7
		case 4: return "Right, facing left, back."; // Prefab5 or Prefab12
		case 5: return "Left, facing you."; // Prefab14
		case 6: return "Top, left, back."; // Prefab16
		}
	}
	else if (tileset == "/Lotus/Levels/Proc/Corpus/CorpusOutpostRescue") // Venus & Pluto
	{
		// 4 cells
		switch (cell)
		{
		case 1: return "Left, back."; // Prefab26
		case 2: return "Left, front."; // Prefab29
		case 3: return "Right, front.";
		case 4: return "Right, back."; // Prefab31
		}
	}
	else if (tileset == "/Lotus/Levels/Proc/Grineer/GrineerAsteroidRescue") // Mercury & Saturn
	{
		// /Lotus/Levels/Grineer/GrnRescue.level, 8 cells
		switch (cell)
		{
		case 2: return "Right corridor, left, back."; // Prefab33
		// case ?: Right corridor, left, front."; // Prefab31 or Prefab36
		case 5: return "Left corridor, right, back."; // Prefab31 or Prefab36
		case 6: return "Left corridor, right, front."; // Prefab34, Prefab36, or Prefab37
		}
	}
	else if (tileset == "/Lotus/Levels/Proc/Grineer/GrineerSettlementRescue") // Mars
	{
		// 9 cells
		switch (cell)
		{
		case 2: return "Left, front."; // Prefab0, Prefab2, Prefab6, Prefab8, or Prefab24
		}
	}
	else if (tileset == "/Lotus/Levels/Proc/Corpus/CorpusGasCityRescue") // Jupiter
	{
		// 4 cells
		switch (cell)
		{
		case 2: return "Bottom, left."; // Prefab0 or Prefab1
		//case ?: return "Bottom, right."; // Prefab0 or Prefab1
		}
	}
	else if (tileset == "/Lotus/Levels/Proc/Corpus/CorpusShipRescue" // Phobos & Neptune
		|| tileset == "/Lotus/Levels/Proc/Transitional/GrineerToCorpusRescue" // in this case, the tile is marked as 'B', not 'O'.
		)
	{
		// /Lotus/Levels/CorpusShip/ObjRescue01.level, 8 cells
		switch (cell)
		{
		case 1: return "Right, 4/4 from front."; // Prefab27
		case 2: return "Right, 3/4 from front.";
		case 3: return "Right, 1/4 from front.";
		case 5: return "Left, 3/4 from front."; // Prefab31
		case 6: return "Left, 2/4 from front."; // Prefab32
		}
	}
	else if (tileset == "/Lotus/Levels/Proc/Grineer/GrineerShipyardsRescue") // Ceres
	{
		// /Lotus/Levels/GrineerShipyards/ShipyardsRescue.level, 4 cells
		switch (cell)
		{
		//case ?: return "Bottom, door 2/4."; // Prefab15 or Prefab18
		case 4: return "Bottom, door 4/4."; // Prefab15 or Prefab18
		}
	}
	else if (tileset == "/Lotus/Levels/Proc/Corpus/CorpusIcePlanetRescue") // Europa
	{
		// /Lotus/Levels/CorpusIcePlanet/IceRescue.level, 5 cells
		switch (cell)
		{
		case 2: return "Top, left."; // Prefab105 or Prefab106
		//case ?: "Right, bottom."; // Prefab105 or Prefab106
		}
	}
	return nullptr;
}
