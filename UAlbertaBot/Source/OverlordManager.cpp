#include "OverlordManager.h"
#include "Common.h"
#include <random>
#include "UnitUtil.h"

using namespace UAlbertaBot;

OverlordManager::OverlordManager()
{
	_overlordScouts.clear();
}

void OverlordManager::update(const BWAPI::Unitset & overlords)
{
	for (auto & overlord : overlords) {

		if (scouting == false)
		{
			_startLocations = BWTA::getStartLocations();
			for (auto i : BWTA::getStartLocations())
				_startStack.push(BWAPI::Position(i->getTilePosition()));
			for (auto i : BWTA::getBaseLocations())
				if (_startLocations.find(i) == _startLocations.end())
					_locationsStack.push(BWAPI::Position(i->getTilePosition()));
			for (auto i : BWTA::getUnwalkablePolygons())
			{
				_unwalkablepolys.push_back(i);
			}
			std::sort(_unwalkablepolys.begin(), _unwalkablepolys.end(), [](BWTA::Polygon* a, BWTA::Polygon* b) {return a->getArea() < b->getArea(); });
				
			_mainOverlord = overlord;
			scouting = true;
		}
		//moveToUnwalkable(overlord);
		if (!_overlordScouts.contains(overlord))
		{
			if (_startStack.size() > 0)
			{
				BWAPI::Position pos = _startStack.top();
				_startStack.pop();
				overlord->move(pos);
			}
			else if (_locationsStack.size() > 0)
			{
				BWAPI::Position pos = _locationsStack.top();
				_locationsStack.pop();
				overlord->move(pos);
			} 
			else
			{
				moveToUnwalkable(overlord);
			}
			_overlordScouts.insert(overlord);
		}
	}

	BWAPI::Unitset cloakedUnits;
	BWAPI::Unitset overlordDetectors;
	// figure out targets
	for (auto & unit : BWAPI::Broodwar->enemy()->getUnits())
	{
		// conditions for targeting
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Lurker ||
			unit->getType() == BWAPI::UnitTypes::Protoss_Dark_Templar ||
			unit->getType() == BWAPI::UnitTypes::Terran_Wraith)
		{
			cloakedUnits.insert(unit);
		}
	}

	for (auto cloakUnit : cloakedUnits)
	{
		BWAPI::Unit closest = *_overlordScouts.begin();
		for (auto overlord : _overlordScouts)
		{
			BWAPI::Position p = overlord->getPosition();
			BWAPI::Position c = closest->getPosition();
			if (c.getDistance(cloakUnit->getPosition()) > p.getDistance(cloakUnit->getPosition()))
			{
				closest = overlord;
			}
		}
		closest->move(cloakUnit->getPosition());
	}

	for (auto overlord : _overlordScouts)
	{
		if (!overlord || !overlord->exists() || !(overlord->getHitPoints() > 0))
			continue;

		BWAPI::Unitset enemyNear;
		MapGrid::Instance().GetUnits(enemyNear, overlord->getPosition(), 400, false, true);
		for (auto unit : enemyNear)
		{
			if (UnitUtil::CanAttackAir(unit))
			{
				BWAPI::Position fleePosition(overlord->getPosition() - unit->getPosition() +overlord->getPosition());
				BWAPI::Broodwar->drawLineMap(overlord->getPosition(), fleePosition, BWAPI::Colors::Cyan);
				overlord->move(fleePosition);
			}
		}
		if (!overlord->isMoving())
			moveToUnwalkable(overlord);
	}


}
void OverlordManager::moveToUnwalkable(const BWAPI::Unit & overlord)
{
	auto polys = BWTA::getUnwalkablePolygons();
	auto poly = *polys.begin();
	for (auto i : polys)
	{
		if (poly->getArea() < i->getArea())
		{
			poly = i;
		}
		/*auto prev = *((*i).begin());
		for (auto next : *i)
		{

			BWAPI::Position p(prev.x * 8, prev.y * 8);
			BWAPI::Position n(next.x * 8, next.y * 8);
			BWAPI::Broodwar->drawLineMap(p, n, BWAPI::Colors::Green);
			prev = next;
		}*/
	}
	int ran = rand() % (_unwalkablepolys.size()-1);
	poly = _unwalkablepolys[ran];
	/*auto prev = *((*poly).begin());
	for (auto next : *poly)
	{

		BWAPI::Position p(prev.x * 8, prev.y * 8);
		BWAPI::Position n(next.x * 8, next.y * 8);
		BWAPI::Broodwar->drawLineMap(p, n, BWAPI::Colors::Red);
		prev = next;
	}*/
	BWAPI::Position gohere(poly->getCenter().x * -8, poly->getCenter().y * -8);
	//BWAPI::Broodwar->drawTextScreen(200, 200, "POLY CENTER X: %d Y:%d",gohere.x,gohere.y);
	overlord->move(gohere);
	//BWAPI::Broodwar->drawLineMap(overlord->getPosition(), gohere, BWAPI::Colors::Cyan);
}

void OverlordManager::moveRandomly(const BWAPI::Unit & overlord)
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<int> randWidth = std::uniform_int_distribution<int>(0, BWAPI::Broodwar->mapWidth() * 32);
	static std::uniform_int_distribution<int> randHeight = std::uniform_int_distribution<int>(0, BWAPI::Broodwar->mapHeight() * 32);
	int width = randWidth(gen);
	int height = randHeight(gen);
	BWAPI::Position pos = BWAPI::Position(width, height);
	
	overlord->move(pos);
	BWAPI::Broodwar->drawLineMap(overlord->getPosition(), pos, BWAPI::Colors::Cyan);
}
