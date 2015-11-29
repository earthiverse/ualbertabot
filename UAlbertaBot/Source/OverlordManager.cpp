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

		if (_overlordScouts.size() == 0)
		{
			_startLocations = BWTA::getStartLocations();
			for (auto i : BWTA::getStartLocations())
				_startStack.push(BWAPI::Position(i->getTilePosition()));
			for (auto i : BWTA::getBaseLocations())
				if (_startLocations.find(i) == _startLocations.end())
					_locationsStack.push(BWAPI::Position(i->getTilePosition()));
				
			_mainOverlord = overlord;
			scouting = true;
		}

		if (!_overlordScouts.contains(overlord))
		{
			if (_startStack.size() > 0)
			{
				BWAPI::Position pos = _startStack.top();
				_startStack.pop();
				overlord->move(pos);
			}
			else if (_startLocations.size() > 0)
			{
				BWAPI::Position pos = _locationsStack.top();
				_locationsStack.pop();
				overlord->move(pos);
			} 
			else
			{
				moveRandomly(overlord);
			}
			_overlordScouts.insert(overlord);
		}
	}

	for (auto overlord : _overlordScouts)
	{
		if (!overlord || !overlord->exists() || !(overlord->getHitPoints() > 0))
		{
			continue;
		}
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
		{
			moveRandomly(overlord);
		}
	}
}
void OverlordManager::moveRandomly(const BWAPI::Unit & overlord)
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<int> randWidth = std::uniform_int_distribution<int>(0, BWAPI::Broodwar->mapWidth()*32);
	static std::uniform_int_distribution<int> randHeight = std::uniform_int_distribution<int>(0, BWAPI::Broodwar->mapHeight()*32);
	int width = randWidth(gen);
	int height = randHeight(gen);
	//int w = rand() % BWAPI::Broodwar->mapWidth()*32;
	//int h = rand() % BWAPI::Broodwar->mapHeight()*32;

	BWAPI::Position pos = BWAPI::Position(width, height);
	overlord->move(pos);
}
