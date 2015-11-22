#include "OverlordManager.h"
#include "Common.h"
#include <random>

using namespace UAlbertaBot;

OverlordManager::OverlordManager()
{
	_overlordScouts.clear();
}


void OverlordManager::update(const BWAPI::Unitset & overlords)
{
	for (auto & overlord : overlords) {
		
		if (!scouting)
		{
			_mainOverlord = overlord;
			_overlordScouts.insert(overlord);
			scouting = true;
		}
		
		if (!_mainOverlord->isMoving())
		{
			BWTA::BaseLocation * enemyBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy());

			if (!enemyBaseLocation) //if not found yet
			{
				for (BWTA::BaseLocation * startLocation : BWTA::getStartLocations())
				{
					// if we haven't explored it yet
					if (!BWAPI::Broodwar->isExplored(startLocation->getTilePosition()))
					{
						_mainOverlord->move(BWAPI::Position(startLocation->getTilePosition()));
					}
				}
			}
			
			if (enemyBaseLocation && !BWAPI::Broodwar->isExplored(enemyBaseLocation->getTilePosition()))
			{
				_mainOverlord->move(BWAPI::Position(enemyBaseLocation->getTilePosition()));
			}
		}

		if (!_overlordScouts.contains(overlord))
		{
			moveRandomly(overlord);
			_overlordScouts.insert(overlord);
		}
	}
}

void OverlordManager::moveRandomly(const BWAPI::Unit & overlord)
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<int> randWidth = std::uniform_int_distribution<int>(0, BWAPI::Broodwar->mapWidth() * 64);
	static std::uniform_int_distribution<int> randHeight = std::uniform_int_distribution<int>(0, BWAPI::Broodwar->mapHeight() * 64);
	int width = randWidth(gen);
	int height = randHeight(gen);

	BWAPI::Position pos = BWAPI::Position(width, height);
	overlord->move(pos);
}
