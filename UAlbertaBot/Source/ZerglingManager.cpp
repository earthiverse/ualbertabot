#include "ZerglingManager.h"
#include "UnitUtil.h"

using namespace UAlbertaBot;

ZerglingManager::ZerglingManager()
{
}

void ZerglingManager::executeMicro(const BWAPI::Unitset & targets)
{
	BWAPI::Broodwar->printf("Zergling!");
}

