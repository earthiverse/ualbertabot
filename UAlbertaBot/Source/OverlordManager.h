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
	std::map<BWAPI::Unit, BWAPI::Position> _overlordpos;
	bool scouting = false;
	BWAPI::Unit _mainOverlord;
	std::vector<BWAPI::Position> _possibleBases;
	std::set < BWTA::BaseLocation *,
		std::less<BWTA::BaseLocation *>,
		std::allocator < BWTA::BaseLocation * >> _startLocations;
	std::stack<BWAPI::Position> _startStack;
	std::stack<BWAPI::Position> _locationsStack;
	void moveRandomly(const BWAPI::Unit & overlord);
	void moveToUnwalkable(const BWAPI::Unit & overlord);
	std::vector<BWTA::Polygon*> _unwalkablepolys;
	
	DetectorManager _detectorManager;

public:

	OverlordManager();

	void update(const BWAPI::Unitset & overlords);
};
}
