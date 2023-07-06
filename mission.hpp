#pragma once

#include <string>

inline std::string current_missionType;
inline std::string current_levelOverride;
inline int hostage_cell;
inline int mission_stage;

/****** Rescue mission cells

Earth - /Lotus/Levels/Proc/Grineer/GrineerGalleonRescue (6):
- Cell 1 (Prefab4 or Prefab7): Top, right, front.
- Cell 4 (Prefab5, Prefab7, or Prefab12): Right, facing left, back.
- Cell 5 (Prefab14): Left, facing you.
- Cell 6 (Prefab16): Top, left, back.

Venus - /Lotus/Levels/Proc/Corpus/CorpusOutpostRescue (4):
- Cell 1: Left, back.
- Cell 2 (Prefab29): Left, front.
- Cell 3: Right, front.
- Cell 4 (Prefab31): Right, back.

Mercury - /Lotus/Levels/Proc/Grineer/GrineerAsteroidRescue (8):
- Cell 2 (Prefab33, Prefab35, or Prefab37): Right corridor, left, back.
- Cell 6 (Prefab34, Prefab36, or Prefab37): Left corridor, right, front.

Mars - /Lotus/Levels/Proc/Grineer/GrineerSettlementRescue (9):
- Cell 2 (Prefab0, Prefab2, Prefab6, Prefab8, or Prefab24): Left, front.

Jupiter - /Lotus/Levels/Proc/Corpus/CorpusGasCityRescue (4):
- Cell 2 (Prefab0 or Prefab1): Bottom, left.
- Cell ? (Prefab1 or Prefab1): Bottom, right.

# Tile /Lotus/Levels/CorpusShip/ObjRescue01.level
Found in:
- /Lotus/Levels/Proc/Corpus/CorpusShipRescue (O) - Phobos
- /Lotus/Levels/Proc/Transitional/GrineerToCorpusRescue (B)
Cells (8):
- Cell 2: Right, 3/4 from front.
- Cell 3: Right, 1/4 from front.
- Cell 6 (Prefab32): Left, 2/4 from front.

*/
