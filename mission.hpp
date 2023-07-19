#pragma once

#include <string>

inline std::string current_missionName;
inline std::string current_missionType;
inline std::string current_levelOverride;
inline int hostage_cell = -1;
inline int mission_stage;

inline bool just_completed_sortie = false;

[[nodiscard]] inline const char* getCellName(const std::string& tileset, int cell)
{
	if (tileset == "/Lotus/Levels/Proc/Grineer/GrineerGalleonRescue") // Earth & Sedna
	{
		// /Lotus/Levels/GrineerGalleon/GrnRescueObjectiveRoom.level, 6 cells
		switch (cell)
		{
		case 1: return "Top, right, front."; // Prefab7
		case 2: return "Right, facing front.";
		case 3: return "Left, facing right, front.";
		case 4: return "Right, facing left, back."; // Prefab5 or Prefab12
		case 5: return "Left, facing front."; // Prefab14
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
		case 1: return "Right corridor, left, front."; // Prefab31 or Prefab36
		case 2: return "Right corridor, left, back."; // Prefab33
		case 3: return "Right corridor, right, front.";
		case 4: return "Right corridor, right, back."; // unconfirmed but what else would it be
		case 5: return "Left corridor, right, back."; // Prefab31 or Prefab36
		case 6: return "Left corridor, right, front."; // Prefab34, Prefab36, or Prefab37
		case 7: return "Left corridor, left, back.";
		case 8: return "Left corridor, left, front.";
		}
	}
	else if (tileset == "/Lotus/Levels/Proc/Grineer/GrineerSettlementRescue") // Mars
	{
		// /Lotus/Levels/GrineerSettlement/CmpRescue.level, 9 cells
		switch (cell)
		{
		case 1: return "Bottom, right.";
		case 2: return "Top left, front."; // Prefab0, Prefab2, Prefab6, Prefab8, or Prefab24
		case 3: return "Back, right.";
		case 4: return "Top right, front."; // Prefab23, Prefab8, Prefab0, or Prefab24
		case 5: return "Bottom, left."; // Prefab27, Prefab12, or Prefab23
		case 6: return "Top right, back.";
		case 7: return "Back, centre."; // Prefab27, Prefab12, Prefab18, Prefab8, or Prefab23
		case 8: return "Back, left.";
		case 9: return "Top left, back.";
		}
	}
	else if (tileset == "/Lotus/Levels/Proc/Corpus/CorpusGasCityRescue") // Jupiter
	{
		// 4 cells
		switch (cell)
		{
		case 1: return "Top, left.";
		case 2: return "Bottom, left."; // Prefab0 or Prefab1
		case 3: return "Top, right."; // unconfirmed but what else would it be
		case 4: return "Bottom, right."; // Prefab0 or Prefab1
		}
	}
	else if (tileset == "/Lotus/Levels/Proc/Grineer/GrineerOceanRescue") // Uranus
	{
		// /Lotus/Levels/GrineerOcean/GrineerOceanRescue.level, 4 cells
		switch (cell)
		{
		case 1: return "Left, left.";
		case 2: return "Left, right."; // Prefab4 or Prefab2
		case 3: return "Right, left";
		case 4: return "Right, right.";
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
		case 4: return "Right, 2/4 from front."; // unconfirmed but what else would it be
		case 5: return "Left, 3/4 from front."; // Prefab31
		case 6: return "Left, 2/4 from front."; // Prefab32
		case 7: return "Left, 4/4 from front.";
		case 8: return "Left, 1/4 from front.";
		}
	}
	else if (tileset == "/Lotus/Levels/Proc/Grineer/GrineerShipyardsRescue" // Ceres
		|| tileset == "/Lotus/Levels/Proc/Grineer/GrineerShipyardsRescueSimplified" // Recall: Ten-Zero Mission 1
		)
	{
		// /Lotus/Levels/GrineerShipyards/ShipyardsRescue.level, 4 cells
		switch (cell)
		{
		case 1: return "Bottom, door 2/4."; // Prefab15
		case 2: return "Bottom, door 1/4."; // Prefab16
		case 3: return "Bottom, door 3/4."; // Prefab17
		case 4: return "Bottom, door 4/4."; // Prefab18
		}
	}
	else if (tileset == "/Lotus/Levels/Proc/Corpus/CorpusIcePlanetRescue") // Europa
	{
		// /Lotus/Levels/CorpusIcePlanet/IceRescue.level, 5 cells
		switch (cell)
		{
		case 1: return "Bottom floor, right."; // Prefab105
		case 2: return "Top floor, left."; // Prefab106
		case 3: return "Top floor, right.";
		case 4: return "Middle floor, right.";
		case 5: return "Middle floor, left."; // unconfirmed but there is no "Bottom floor, left." so this is the only remaining option
		}
	}
	else if (tileset == "/Lotus/Levels/Proc/Infestation/InfestedCorpusShipRescue") // Eris
	{
		// /Lotus/Levels/InfestedCorpus/InfestedRescueEscapePods.level, 7 cells
		switch (cell)
		{
		case 1: return "Top left, left."; // Prefab13, Prefab11, or Prefab16
		case 2: return "Top right, left."; // Prefab16, Prefab12, or Prefab14
		case 3: return "Top right, right/centre."; // Prefab12, Prefab13, or Prefab14
		case 4: return "Top left, right."; // Prefab12, Prefab18, or Prefab14
		case 5: return "Bottom left, left/centre.";
		case 6: return "Bottom left, right."; // Prefab17, Prefab12, or Prefab16
		case 7: return "Bottom right, left."; // Prefab18, Prefab17, or Prefab11
		}
	}
	else if (tileset == "/Lotus/Levels/Proc/Orokin/OrokinMoonRescue") // Lua
	{
		// /Lotus/Levels/OrokinMoon/MoonObjRescue01.level, 5 cells
		switch (cell)
		{
		case 1: return "Console 2/5."; // Prefab14, Prefab10, or Prefab12
		case 2: return "Console 3/5."; // Prefab12, Prefab11, Prefab14, or Prefab10
		case 3: return "Console 4/5."; // Prefab10, Prefab11, or Prefab12
		case 4: return "Console 1/5."; // Prefab13
		case 5: return "Console 5/5."; // Prefab10, Prefab14
		}
	}
	else if (tileset == "/Lotus/Levels/Proc/Grineer/GrineerFortressRescue") // Kuva Fortress
	{
		// /Lotus/Levels/GrineerFortress/FortObjRescue.level, 4 cells
		switch (cell)
		{
		case 1: return "Left, front.";
		case 2: return "Right, front.";
		case 3: return "Right, back.";
		case 4: return "Back, left."; // Prefab33, Prefab31, Prefab30, or Prefab29
		}
	}
	return nullptr;
}
