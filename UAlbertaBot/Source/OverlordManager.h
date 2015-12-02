#pragma once

#include "Common.h"
#include "MicroManager.h"
#include <stack>
#include "DetectorManager.h"

namespace UAlbertaBot
{
class OverlordManager
{	
	BWAPI::Unitset _overlordScouts;
	bool scouting = false;
	BWAPI::Unit _mainOverlord;
	std::vector<BWAPI::Position> _possibleBases;
	std::set < BWTA::BaseLocation *,
		std::less<BWTA::BaseLocation *>,
		std::allocator < BWTA::BaseLocation * >> _startLocations;
	std::stack<BWAPI::Position> _startStack;
	std::stack<BWAPI::Position> _locationsStack;
	std::vector<BWTA::Polygon*> _unwalkablePolys;

	void moveRandomly(const BWAPI::Unit & overlord);
	void moveToUnwalkable(const BWAPI::Unit & overlord);

public:

	OverlordManager();

	void update(const BWAPI::Unitset & overlords);
};
}
