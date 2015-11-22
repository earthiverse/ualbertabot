#pragma once

#include "Common.h"
#include "MicroManager.h"

namespace UAlbertaBot
{
class OverlordManager
{	
	BWAPI::Unitset _overlordScouts;
	bool scouting = false;
	BWAPI::Unit _mainOverlord;
	
	void moveRandomly(const BWAPI::Unit & overlord);

public:

	OverlordManager();

	void update(const BWAPI::Unitset & overlords);
};
}